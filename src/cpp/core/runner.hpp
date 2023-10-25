#ifndef runner_hpp
#define runner_hpp

#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <filesystem>
#include <functional>

#include "../interfaces/system.hpp"
#include "../interfaces/config.hpp"
#include "../interfaces/log.hpp"

namespace runner
{

  const std::filesystem::path DEFAULT_ROOTBACK = "/succ/rootback";

  enum run_mode_t
  {
    RUN_MODE_PERMANENT,
    RUN_MODE_TEMPORARY,
  };

  void run(logging::logger_t &logger, run_mode_t run_mode,
           std::filesystem::path sysroot,
           std::filesystem::path rootback,
           const std::vector<std::filesystem::path> &persistent_directories,
           std::optional<std::filesystem::path> executable)
  {
    std::vector<std::function<void()>> rollback_stack;
    auto roll_one_back = [&rollback_stack]()
    {
      (rollback_stack.back())();
      rollback_stack.pop_back();
    };

    auto roll_all_back = [&rollback_stack]()
    {
      for (auto rollback = rollback_stack.rbegin(); rollback != rollback_stack.rend(); rollback++)
        (*rollback)();
    };

    try
    {

      if (run_mode == run_mode_t::RUN_MODE_TEMPORARY)
      {
        logger.info() << "Creating new mount namespace..." << std::endl;
        sys::mnt::new_namespace();
        for (const auto &m : sys::mnt::list())
          sys::mnt::make_private(m.target);
      }

      if (!std::filesystem::exists(sysroot))
      {
        throw std::runtime_error("sysroot does not exist.");
      }

      std::filesystem::path tmprootback = "/tmprootback";
      if (std::filesystem::exists(sysroot / tmprootback.relative_path()))
      {
        throw std::runtime_error("Temporary rootback directory already exists. Please remove it.");
      }
      if (!std::filesystem::create_directories(sysroot / tmprootback.relative_path()))
      {
        throw std::runtime_error("Cannot create temporary rootback directory.");
      }
      rollback_stack.push_back([&sysroot, tmprootback]()
                               { if (std::filesystem::exists(sysroot / tmprootback.relative_path()))
                                  std::filesystem::remove_all(sysroot / tmprootback.relative_path()); });

      logger.info() << "Preparing persistent directories..." << std::endl;
      for (const auto &p : persistent_directories)
      {
        if (!std::filesystem::exists(p))
        {
          throw std::runtime_error("Persistent directory " + p.string() + " does not exist.");
        }

        sys::mnt::bind(p, p);
        rollback_stack.push_back([p]()
                                 { sys::mnt::detach(p); });
      }

      std::vector<std::string> migrating_mounts;
      logger.info() << "Registering mountpoints to move..." << std::endl;
      for (const auto &mountpoint : sys::mnt::list())
      {
        if (mountpoint.target == "/")
          continue;
        if (find_if(migrating_mounts.begin(), migrating_mounts.end(), [&mountpoint](std::string &other)
                    { return mountpoint.target.rfind(other) == 0; }) != migrating_mounts.end())
          continue;
        else
        {
          migrating_mounts.push_back(mountpoint.target);
          std::remove_if(migrating_mounts.begin(), migrating_mounts.end(), [&mountpoint](std::string &other)
                         { return other.rfind(mountpoint.target) == 0; });
        }
      }

      for (const auto &m : migrating_mounts)
        if (!std::filesystem::exists(sysroot / m.substr(1)))
        {
          logger.warn() << "Warning: mountpoint " << m << " does not exist. Creating..." << std::endl;
          if (!std::filesystem::create_directories(sysroot / m.substr(1)))
          {
            throw std::runtime_error("Cannot create mountpoint " + m + ".");
          }
        }

      logger.info() << "Binding sysroot to itself..." << std::endl;
      sys::mnt::bind(sysroot, sysroot);
      rollback_stack.push_back([&sysroot]()
                               { sys::mnt::detach(sysroot); });

      logger.info() << "Setting root..." << std::endl;
      sys::pivot_root(sysroot, sysroot / tmprootback.relative_path());
      rollback_stack.push_back([tmprootback, &sysroot, &logger]()
                               { sys::pivot_root(tmprootback, tmprootback / sysroot.relative_path()); });

      // TODO: make pivot+chdir atomic
      std::string previous_cwd = std::filesystem::current_path().string();
      logger.info() << "Changing directory to root..." << std::endl;
      std::filesystem::current_path("/");
      rollback_stack.push_back([previous_cwd]()
                               { std::filesystem::current_path(previous_cwd); });

      for (const std::string &mountpoint : migrating_mounts)
      {
        logger.info() << "Moving mountpoint " << mountpoint << "..." << std::endl;
        bool use_bind = false;
        if (!use_bind)
          try
          {
            sys::mnt::move(tmprootback / mountpoint.substr(1), mountpoint);
            rollback_stack.push_back([tmprootback, mountpoint]()
                                     { sys::mnt::move(mountpoint, tmprootback / mountpoint.substr(1)); });
          }
          catch (std::runtime_error &e)
          {
            logger.warn() << "Warning: Cannot move mountpoint " << mountpoint << std::endl;
            use_bind = true;
          }

        if (use_bind)
        {
          sys::mnt::bind(tmprootback / mountpoint.substr(1), mountpoint);
          rollback_stack.push_back([mountpoint]()
                                   { sys::mnt::detach(mountpoint); });
        }
      }

      logger.info() << "Moving tmprootback..." << std::endl;
      if (!std::filesystem::exists(rootback))
        throw std::runtime_error("Rootback directory " + rootback.string() + " does not exist.");
      sys::mnt::move(tmprootback, rootback);
      std::filesystem::remove(tmprootback);
      rollback_stack.push_back([&rootback, tmprootback]()
                               {
        std::filesystem::create_directory(tmprootback);
        sys::mnt::move(rootback, tmprootback); });

      if (executable)
      {
        logger.info() << "Executing " << executable.value() << "..." << std::endl;
        bool replace = run_mode == run_mode_t::RUN_MODE_PERMANENT;
        int result = sys::execute(executable.value(), {}, false);
        // it will be unreachable for replace == true
        if (result != 0)
          logger.warn() << "Warning: executable exited with code " << result << std::endl;
      }
    }
    catch (const std::exception &e)
    {
      logger.error() << "Error: " << e.what() << std::endl;
      logger.info() << "Rolling back..." << std::endl;
      roll_all_back();
      logger.info() << "Rollback complete." << std::endl;
      throw e;
    }

    logger.info() << "Rolling back..." << std::endl;
    roll_all_back();
    logger.info() << "Rollback complete." << std::endl;
  }
}

#endif