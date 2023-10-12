#ifndef cli_hpp
#define cli_hpp

#include <string>
#include <optional>
#include <stdexcept>
#include <iostream>

struct Arguments
{
  enum
  {
    NORMAL,
    HELP,
    TEST
  } mode;
  std::string plan_path = "/succ/directories.yml";
  std::string log_path = "/succ/succ.log";
  std::string newroot_path = "/succ/new";
  std::optional<std::string> next_executable_path = "/bin/sh";

  std::string temproot_path = "/tmp/rootfs_succ";
  std::string oldroot_path_rt_temproot = "/old";
};

Arguments parse_args(int argc, char **argv)
{
  if (argc <= 1)
    return Arguments{.mode = Arguments::NORMAL};
  else if (argc == 2)
  {
    if (string(argv[1]) == "--help" || string(argv[1]) == "-h")
      return Arguments{.mode = Arguments::HELP};
    else if (string(argv[1]) == "--test" || string(argv[1]) == "-t")
      return Arguments{.mode = Arguments::TEST};
    else
      throw runtime_error("Error: Invalid argument.");
  }
  else
    throw runtime_error("Error: Too many arguments.");
}

void print_help()
{
  cout << "Usage: succ [OPTION]" << endl;
  cout << "  -h, --help     Display this help and exit." << endl;
  cout << "  -t, --test     Run in test mode." << endl;
}


#endif