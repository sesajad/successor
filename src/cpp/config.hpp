#ifndef cli_hpp
#define cli_hpp

#include <string>
#include <optional>
#include <stdexcept>
#include <iostream>
#include <unistd.h>

struct config_t
{
  enum mode_t
  {
    MODE_NORMAL,
    MODE_HELP,
    MODE_TEST,
    MODE_VERSION
  };
  enum verbosity_t
  {
    VERBOSITY_QUIET,
    VERBOSITY_NORMAL,
    VERBOSITY_VERBOSE
  };

  mode_t mode;
  verbosity_t verbosity;
  std::string sysroot_path;
  std::string rootback_path;
  std::string next_executable_path;
};

config_t parse_args(int argc, char **argv)
{
  std::optional<config_t::mode_t> mode;
  std::optional<config_t::verbosity_t> verbosity;
  std::optional<std::string> sysroot_path;
  std::optional<std::string> rootback_path;
  std::optional<std::string> next_executable_path;
  for (int i = 1; i < argc; i++)
  {
    if (argv[i] == "-t" || argv[i] == "--wet-test")
    {
      if (mode)
        throw std::runtime_error("Cannot specify multiple modes");
      mode = config_t::MODE_TEST;
    }
    else if (argv[i] == "-h" || argv[i] == "--help")
    {
      if (mode)
        throw std::runtime_error("Cannot specify multiple modes");
      mode = config_t::MODE_HELP;
    }
    else if (argv[i] == "-V" || argv[i] == "--version")
    {
      if (mode)
        throw std::runtime_error("Cannot specify multiple modes");
      mode = config_t::MODE_VERSION;
    }
    else if (argv[i] == "-v" || argv[i] == "--verbose")
    {
      if (verbosity)
        throw std::runtime_error("Cannot specify multiple verbosity levels");
      verbosity = config_t::VERBOSITY_VERBOSE;
    }
    else if (argv[i] == "-q" || argv[i] == "--quiet")
    {
      if (verbosity)
        throw std::runtime_error("Cannot specify multiple verbosity levels");
      verbosity = config_t::VERBOSITY_QUIET;
    }
    else if (argv[i] == "--exec")
    {
      if (i + 1 >= argc)
        throw std::runtime_error("Expected argument after --exec");
      if (next_executable_path)
        throw std::runtime_error("Cannot specify multiple executables");
      next_executable_path = argv[++i];
    }
    else if (argv[i] == "--rootback")
    {
      if (i + 1 >= argc)
        throw std::runtime_error("Expected argument after --rootback");
      if (rootback_path)
        throw std::runtime_error("Cannot specify multiple rootback paths");
      rootback_path = argv[++i];
    }
    else
    {
      if (sysroot_path)
        throw std::runtime_error("Cannot specify multiple sysroot paths");
      sysroot_path = argv[i];
    }
  }
  return config_t{
      mode.value_or(config_t::MODE_NORMAL),
      verbosity.value_or(config_t::VERBOSITY_NORMAL),
      sysroot_path.value(),
      rootback_path.value_or("/rootback"),
      next_executable_path.value_or(getpid() == 1 ? "/bin/init" : "/bin/sh")};
}

void print_help()
{
  std::cout << R"(
Operation Modes:
  -t --wet-test Run a wet test (switch to root directory and return)
  -h --help     Show this screen.
  -V --version  Show version.
  
Options:
  -v --verbose  Verbose output
  -q --quiet    Quiet output
  --exec EXECUTABLE
                Executable to run afterwards
                Default: /bin/init if pid 1, otherwise /bin/sh
  --rootback ROOTBACK
                Rootback directory in the new root
                Default: /rootback
  )" << std::endl;
}

void print_version()
{
  std::cout << "version 0.1.0" << std::endl;
}

#endif