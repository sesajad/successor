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

  void run(const logging::logger_t &logger, run_mode_t run_mode,
           std::filesystem::path sysroot,
           std::optional<std::filesystem::path> rootback,
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
        sys::new_mount_namespace();
      }

      if (!std::filesystem::exists(sysroot))
      {
        throw std::runtime_error("sysroot does not exist.");
      }

      std::filesystem::path tmprootback = "/tmprootback";
      if (std::filesystem::exists(sysroot / tmprootback))
      {
        throw std::runtime_error("Temporary rootback directory already exists. Please remove it.");
      }
      if (!std::filesystem::create_directories(sysroot / tmprootback))
      {
        throw std::runtime_error("Cannot create temporary rootback directory.");
      }

      logger.info << "Preparing persistent directories..." << std::endl;
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
      logger.info << "Registering mountpoints to move..." << std::endl;
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
        if (!std::filesystem::exists(sysroot / m))
        {
          logger.warn << "Warning: mountpoint " << m << " does not exist. Creating...";
          if (!std::filesystem::create_directories(sysroot / m))
          {
            throw std::runtime_error("Cannot create mountpoint " + m + ".");
          }
        }

      logger.info << "Binding sysroot to itself..." << std::endl;
      sys::mnt::bind(sysroot, sysroot);
      rollback_stack.push_back([&sysroot]()
                               { sys::mnt::detach(sysroot); });

      logger.info << "Setting root..." << std::endl;
      sys::pivot_root(sysroot, sysroot / tmprootback);
      rollback_stack.push_back([&tmprootback, &sysroot]()
                               { sys::pivot_root(tmprootback, tmprootback / sysroot); });

      for (const std::string &mountpoint : migrating_mounts)
      {
        logger.info << "Moving mountpoint " << mountpoint << "..." << std::endl;
        bool use_bind = false;
        if (!use_bind)
          try
          {
            sys::mnt::move(tmprootback / mountpoint, mountpoint);
            rollback_stack.push_back([&tmprootback, mountpoint]()
                                     { sys::mnt::move(mountpoint, tmprootback / mountpoint); });
          }
          catch (std::runtime_error &e)
          {
            logger.warn << "Warning: Cannot move mountpoint " << mountpoint << std::endl;
            use_bind = true;
          }

        if (use_bind)
          sys::mnt::bind(tmprootback / mountpoint, mountpoint);
        rollback_stack.push_back([mountpoint]()
                                 { sys::mnt::detach(mountpoint); });
      }

      if (rootback.has_value())
      {
        logger.info << "Moving tmprootback..." << std::endl;
        if (!std::filesystem::exists(rootback.value()))
        {
          logger.warn << "Warning: rootback directory does not exist. Creating..." << std::endl;
          if (!std::filesystem::create_directories(rootback.value()))
          {
            throw std::runtime_error("Cannot create rootback directory.");
          }
        }
        sys::mnt::move(tmprootback, rootback.value());
      } else {
        logger.info << "Unmounting tmprootback..." << std::endl;
        sys::mnt::detach(tmprootback);
      }
      std::filesystem::remove(tmprootback);

      std::string previous_cwd = std::filesystem::current_path().string();
      logger.info << "Changing directory to root..." << std::endl;
      std::filesystem::current_path("/");
      rollback_stack.push_back([&previous_cwd]()
                               { std::filesystem::current_path(previous_cwd); });

      if (executable)
      {
        logger.info << "Executing " << executable.value() << "..." << std::endl;
        if (run_mode == run_mode_t::RUN_MODE_PERMANENT)
        {
          sys::execute_replace(executable.value());
        }
        else
        {
          int result = sys::execute(executable.value());
          if (result != 0)
          {
            throw std::runtime_error("Executable returned non-zero exit code.");
          }
        }
      }
    }
    catch (std::runtime_error &e)
    {
      logger.err << "Error: " << e.what() << std::endl;
      logger.info << "Rolling back..." << std::endl;
      roll_all_back();
      throw e;
    }
  }
}

#endif