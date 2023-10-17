#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <functional>

#include "mount.hpp"
#include "system.hpp"
#include "config.hpp"

int main(int argc, char **argv)
{
  config_t args;
  try
  {
    args = parse_args(argc, argv);
  }
  catch (std::runtime_error &e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    print_help();
    return 1;
  }

  if (args.mode == config_t::MODE_HELP)
  {
    print_help();
    return 0;
  }
  else if (args.mode == config_t::MODE_VERSION)
  {
    print_version();
    return 0;
  }

  std::string sysroot = args.sysroot_path;
  std::string rootback = args.rootback_path;
  std::string rootback_rt_root = sysroot + rootback;

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

  std::vector<std::string> mountpoints;

  std::cout << "Registering mountpoints to move..." << std::endl;
  for (const auto &mountpoint : mnt::list(rootback + "/proc/mounts"))
  {
    if (mountpoint.target == "/" ||
        mountpoint.target == rootback)
      continue;
    if (find_if(mountpoints.begin(), mountpoints.end(), [&mountpoint](std::string &other)
                { return mountpoint.target.rfind(other) == 0; }) != mountpoints.end())
      continue;
    else
    {
      mountpoints.push_back(mountpoint.target);
      std::remove_if(mountpoints.begin(), mountpoints.end(), [&mountpoint](std::string &other)
                     { return other.rfind(mountpoint.target) == 0; });
    }
  }

  if (!std::filesystem::exists(rootback_rt_root))
  {
    std::cout << "Warning: rootback directory does not exist. Creating...";
    if (!std::filesystem::create_directories(rootback_rt_root))
    {
      std::cerr << "Error: Cannot create rootback directory." << std::endl;
      roll_all_back();
      exit(1);
    }
  }

  for (const auto &mountpoint : mountpoints)
    if (!std::filesystem::exists(sysroot + mountpoint))
    {
      std::cout << "Warning: mountpoint " << mountpoint << " does not exist. Creating...";
      if (!std::filesystem::create_directories(sysroot + mountpoint))
      {
        std::cerr << "Error: Cannot create mountpoint " << mountpoint << "." << std::endl;
        roll_all_back();
        exit(1);
      }
    }

  std::cout << "Binding sysroot to itself..." << std::endl;
  if (!mnt::attach_bind(sysroot, sysroot))
  {
    std::cerr << "Error: Cannot attach sysroot to itself." << std::endl;
    roll_all_back();
    exit(1);
  }
  else
    rollback_stack.push_back([&sysroot]()
                             { mnt::detach(sysroot); });

  std::cout << "Pivoting root..." << std::endl;
  if (!sys::pivot_root(sysroot, rootback_rt_root))
  {
    std::cerr << "Error: Cannot pivot root." << std::endl;
    roll_all_back();
    exit(1);
  }
  else
    rollback_stack.push_back([&rootback, &sysroot]()
                             { sys::pivot_root(rootback, rootback + sysroot); });

  for (const std::string &mountpoint : mountpoints)
  {
    std::cout << "Moving mountpoint " << mountpoint << "..." << std::endl;
    bool use_bind = false;
    if (!use_bind)
      if (!mnt::move(mountpoint, mountpoint.substr(rootback.size())))
      {
        std::cerr << "Warning: Cannot move mountpoint " << mountpoint << std::endl;
        use_bind = true;
      }
      else
        rollback_stack.push_back([&rootback, mountpoint]()
                                 { mnt::move(mountpoint.substr(rootback.size()), mountpoint); });

    if (use_bind)
      if (!mnt::attach_bind(mountpoint, mountpoint.substr(rootback.size())))
      {
        std::cerr << "Error: Cannot attach mountpoint " << mountpoint << std::endl;
        roll_all_back();
        exit(1);
      }
      else
        rollback_stack.push_back([&rootback, mountpoint]()
                                 { mnt::detach(mountpoint.substr(rootback.size())); });
  }

  std::cout << "Changing directory to root..." << std::endl;
  if (!sys::change_dir("/"))
  {
    std::cerr << "Error: Cannot change directory to root." << std::endl;
    roll_all_back();
    exit(1);
  }
  else
  {
    rollback_stack.push_back([&rootback]()
                             { sys::change_dir(rootback); });
  }

  if (args.mode == config_t::MODE_TEST)
    roll_all_back();
  else
    sys::execute(args.next_executable_path);
}