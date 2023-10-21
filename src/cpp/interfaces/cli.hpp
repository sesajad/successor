#ifndef cli_hpp
#define cli_hpp

#include <string>
#include <filesystem>
#include <iostream>
#include <optional>
#include "../core/data.hpp"

const std::map<std::string, std::string> HELP_TEXTS{
    {"build", R"(
successor build [--name | -n NAME] [--version | -v VERSION] SOURCE

Description:
Builds a new image from the specified source directory. It uses buildah, docker or podman to build the image.

Options:
--name | -n NAME
The name of the image to build. If not specified, the default image from the config file is used.
--version | -v VERSION
The version of the image to build. If not specified, the latest version is used.

Arguments:
SOURCE
The source directory to build the image from.
)"},
    {"list", R"(
successor list

Description:
Lists all images and their versions, as well as the current and the next image.
)"},
    {"remove", R"(
First Form:
successor remove [--name | -n NAME] --version | -v VERSION

Description:
Removes the specified build from the successor inventory.

Options:
--name | -n NAME
The name of the image to remove. If not specified, the default image from the config file is used.
--version | -v VERSION
The version of the image to remove.

Second Form:
successor remove --unused [--name | -n NAME]

Description:
Removes all of the version, excluding the latest and running one (if any), of the specified image from the successor inventory.

Options:
--name | -n NAME
The name of the image to remove. If not specified, the default image from the config file is used.
)"},
    {"run", R"(
successor run [--name | -n NAME] [--version | -v VERSION] [--persistent-directory | -p DIRECTORY]... [--exec | -e EXECUTABLE] [--replace] [--enable-logging]

Description:
Runs the specified image.

Options:

--name | -n NAME
The name of the image to run. If not specified, the default image from the config file is used.

--version | -v VERSION
The version of the image to run. If not specified, the default version from the config file is used.

--default-persistent-directories | -dp
If specified, the default persistent directories from the config file will be added.

--persistent-directory | -p DIRECTORY
A directory that is shared between the root filesystem and the successor OS.

--exec | -e EXECUTABLE
The executable to run. If not specified, the default executable from the config file is used.

--replace
If specified, the running successor OS will replace the current OS. (use with caution)

--enable-logging
If specified, the successor OS will collect logs.
)"},
    {"logs", R"(
successor logs [--index | -i INDEX]

Description:
Prints the logs of the specified boot.

Options:
--index | -i INDEX
The index of the boot to print the logs of. 1 indicates the current boot, 2 the previous one, and so on. If not specified, the current boot is used.
)"},
    {"", R"(
successor v0.2.0
successor -h | --help
successor COMMAND [OPTIONS]

Commands:
build
list
logs
remove
run

You can use `successor COMMAND --help` to get more information about a specific command.
)"}};

struct help_cmd_t
{
  std::optional<std::string> command;
};

struct build_cmd_t
{
  std::optional<std::string> image;
  std::optional<version_t> version;
  std::filesystem::path source;
};

std::variant<build_cmd_t, help_cmd_t> parse_build_cmd(int argc, char **argv)
{
  build_cmd_t cmd;
  bool source_specified = false;

  for (int i = 1; i < argc; i++)
  {
    std::string arg = argv[i];
    if (arg == "--name" || arg == "-n")
    {
      if (i + 1 >= argc)
        throw std::runtime_error("No image name specified.");
      if (!std::regex_match(argv[i + 1], IMAGE_NAME_REGEX))
        throw std::runtime_error("Invalid image name.");
      if (cmd.image.has_value())
        throw std::runtime_error("Image name already specified.");
      cmd.image = argv[i + 1];
      i++;
    }
    else if (arg == "--version" || arg == "-v")
    {
      if (i + 1 >= argc)
        throw std::runtime_error("No image version specified.");
      if (cmd.version.has_value())
        throw std::runtime_error("Image version already specified.");
      if (std::string(argv[i + 1]) == "latest")
        cmd.version = version_latest;
      else
        cmd.version = std::stoi(argv[i + 1]);
      i++;
    }
    else if (arg == "--help" || arg == "-h")
    {
      return help_cmd_t{.command = "build"};
    }
    else
    {
      if (source_specified)
        throw std::runtime_error("Source directory already specified.");
      source_specified = true;
      cmd.source = argv[i];
    }
  }

  if (!source_specified)
    throw std::runtime_error("No source directory specified.");
  return cmd;
}

struct list_cmd_t
{
};

std::variant<list_cmd_t, help_cmd_t> parse_list_cmd(int argc, char **argv)
{
  for (int i = 1; i < argc; i++)
    if (std::string(argv[i]) == "--help" || std::string(argv[i]) == "-h")
      return help_cmd_t{.command = "list"};
    else
      throw std::runtime_error("Invalid argument.");

  return list_cmd_t();
}

struct logs_cmd_t
{
  std::optional<int> index;
};

std::variant<logs_cmd_t, help_cmd_t> parse_logs_cmd(int argc, char **argv)
{
  logs_cmd_t cmd;

  for (int i = 1; i < argc; i++)
  {
    std::string arg = argv[i];
    if (arg == "--index" || arg == "-i")
    {
      if (i + 1 >= argc)
        throw std::runtime_error("No index specified.");
      if (cmd.index.has_value())
        throw std::runtime_error("Index already specified.");
      cmd.index = std::stoi(argv[i + 1]);
      i++;
    }
    else if (arg == "--help" || arg == "-h")
    {
      return help_cmd_t{.command = "logs"};
    }
    else
    {
      throw std::runtime_error("Invalid argument.");
    }
  }

  return cmd;
}

struct remove_specific_cmd_t
{
  std::string image;
  version_t version;
};

struct remove_unused_cmd_t
{
  std::string image;
};

std::variant<remove_specific_cmd_t, remove_unused_cmd_t, help_cmd_t> parse_remove_cmd(int argc, char **argv)
{
  bool has_unused = false;
  std::optional<std::string> image;
  std::optional<version_t> version;

  for (int i = 1; i < argc; i++)
  {
    std::string arg = argv[i];
    if (arg == "--name" || arg == "-n")
    {
      if (i + 1 >= argc)
        throw std::runtime_error("No image name specified.");
      if (!std::regex_match(argv[i + 1], IMAGE_NAME_REGEX))
        throw std::runtime_error("Invalid image name.");
      if (image.has_value())
        throw std::runtime_error("Image name already specified.");
      image = argv[i + 1];
      i++;
    }
    else if (arg == "--version" || arg == "-v")
    {
      if (i + 1 >= argc)
        throw std::runtime_error("No image version specified.");
      if (version.has_value())
        throw std::runtime_error("Image version already specified.");
      if (std::string(argv[i + 1]) == "latest")
        version = version_latest;
      else
        version = std::stoi(argv[i + 1]);
      i++;
    }
    else if (arg == "--unused")
    {
      if (has_unused)
        throw std::runtime_error("Unused already specified.");
      has_unused = true;
    }
    else if (arg == "--help" || arg == "-h")
    {
      return help_cmd_t{.command = "remove"};
    }
    else
    {
      throw std::runtime_error("Invalid argument.");
    }
  }
  if (has_unused)
  {
    if (version.has_value())
      throw std::runtime_error("Version cannot be specified with unused.");
    if (image.has_value())
      return remove_unused_cmd_t{.image = image.value()};
    else
      throw std::runtime_error("No image name specified.");
  }
  else
  {
    if (!version.has_value())
      throw std::runtime_error("No version specified.");
    if (!image.has_value())
      throw std::runtime_error("No image name specified.");
    return remove_specific_cmd_t{.image = image.value(), .version = version.value()};
  }
}

struct run_cmd_t
{
  bool replace;
  bool enable_logging;
  std::optional<std::string> image;
  std::optional<version_t> version;
  bool add_default_persistent_directories;
  std::vector<std::filesystem::path> persistent_directories;
  std::optional<std::filesystem::path> executable;
};

std::variant<run_cmd_t, help_cmd_t> parse_run_cmd(int argc, char **argv)
{
  run_cmd_t cmd;

  for (int i = 1; i < argc; i++)
  {
    std::string arg = argv[i];
    if (arg == "--name" || arg == "-n")
    {
      if (i + 1 >= argc)
        throw std::runtime_error("No image name specified.");
      if (!std::regex_match(argv[i + 1], IMAGE_NAME_REGEX))
        throw std::runtime_error("Invalid image name.");
      if (cmd.image.has_value())
        throw std::runtime_error("Image name already specified.");
      cmd.image = argv[i + 1];
      i++;
    }
    else if (arg == "--version" || arg == "-v")
    {
      if (i + 1 >= argc)
        throw std::runtime_error("No image version specified.");
      if (cmd.version.has_value())
        throw std::runtime_error("Image version already specified.");
      if (std::string(argv[i + 1]) == "latest")
        cmd.version = version_latest;
      else
        cmd.version = std::stoi(argv[i + 1]);
      i++;
    }
    else if (arg == "--default-persistent-directories" || arg == "-dp")
    {
      if (cmd.add_default_persistent_directories)
        throw std::runtime_error("Default persistent directories already specified.");
      cmd.add_default_persistent_directories = true;
    }
    else if (arg == "--persistent-directory" || arg == "-p")
    {
      if (i + 1 >= argc)
        throw std::runtime_error("No persistent directory specified.");
      cmd.persistent_directories.push_back(argv[i + 1]);
      i++;
    }
    else if (arg == "--exec" || arg == "-e")
    {
      if (i + 1 >= argc)
        throw std::runtime_error("No executable specified.");
      if (cmd.executable.has_value())
        throw std::runtime_error("Executable already specified.");
      cmd.executable = argv[i + 1];
      i++;
    }
    else if (arg == "--replace")
    {
      if (cmd.replace)
        throw std::runtime_error("Replace already specified.");
      cmd.replace = true;
    }
    else if (arg == "--enable-logging")
    {
      if (cmd.enable_logging)
        throw std::runtime_error("Enable logging already specified.");
      cmd.enable_logging = true;
    }
    else if (arg == "--help" || arg == "-h")
    {
      return help_cmd_t{.command = "run"};
    }
    else
    {
      throw std::runtime_error("Invalid argument.");
    }
  }

  return cmd;
}

typedef std::variant<build_cmd_t, list_cmd_t, logs_cmd_t, remove_specific_cmd_t, remove_unused_cmd_t, run_cmd_t, help_cmd_t> cmd_t;


template <class... Fs>
struct overloaded : Fs...
{
  using Fs::operator()...;
};
template <class... Fs>
overloaded(Fs...) -> overloaded<Fs...>;

cmd_t parse_cmd(int argc, char **argv)
{
  if (argc < 2)
  {
    throw std::runtime_error("No command specified.");
  }

  std::string command = argv[1];

  if (command == "-h" || command == "--help")
    return help_cmd_t();

  if (command == "build")
    return std::visit([](auto &&arg) -> cmd_t
                      { return arg; },
                      parse_build_cmd(argc - 1, &argv[1]));
  else if (command == "list")
  {
    return std::visit([](auto &&arg) -> cmd_t
                      { return arg; },
                      parse_list_cmd(argc - 1, &argv[1]));
  }
  else if (command == "logs")
    return std::visit([](auto &&arg) -> cmd_t
                      { return arg; },
                      parse_logs_cmd(argc - 1, &argv[1]));
  else if (command == "remove")
    return std::visit([](auto &&arg) -> cmd_t
                      { return arg; },
                      parse_remove_cmd(argc - 1, &argv[1]));
  else if (command == "run")
    return std::visit([](auto &&arg) -> cmd_t
                      { return arg; },
                      parse_run_cmd(argc - 1, &argv[1]));
  else
    throw std::runtime_error("Invalid command.");
}

#endif