#include <iostream>

#include "interfaces/cli.hpp"
#include "interfaces/config.hpp"

#include "core/inventory.hpp"
#include "core/runner.hpp"

int main(int argc, char **argv)
{
  cmd_t cmd = help_cmd_t{};
  try
  {
    cmd = parse_cmd(argc, argv);
  }
  catch (const std::exception &e)
  {
    std::cout << e.what() << std::endl;
    std::cout << HELP_TEXTS.at("") << std::endl;
    return 1;
  }

  config_t config;
  try
  {
    config = load_config();
  }
  catch (const std::exception &e)
  {
    std::cout << e.what() << std::endl;
    std::cout << "Unable to load config file." << std::endl;
    return 1;
  }

  try
  {
    std::visit(
        overloaded{[&config](build_cmd_t &cmd)
                   {
                     if (!cmd.image.has_value() && !config.default_image_name.has_value())
                       throw std::runtime_error("No image name provided");
                     entity_t entity = inventory::resolve(
                         cmd.image.value_or(config.default_image_name.value_or("")),
                         cmd.version.value_or(version_latest));
                     entity.version++;
                     std::cout << "Building image " << entity.name << ":" << entity.version << std::endl;
                     inventory::build(entity, cmd.source);
                   },
                   [&config](list_cmd_t &cmd)
                   {
                     auto current = inventory::current();
                     for (auto &image : inventory::list_images())
                     {
                       std::cout << "Image Name: " << image << std::endl;
                       for (auto &version : inventory::list_versions(image))
                       {
                         std::cout << "  Version: " << version;
                         if (entity_t{image, version} == inventory::current())
                           std::cout << " (current)";
                         if (config.default_image_name)
                           if (entity_t{image, version} == inventory::resolve(config.default_image_name.value(), config.default_image_version.value_or(version_latest)))
                             std::cout << " (next)";
                          std::cout << std::endl;
                       }
                     }
                   },
                   [](logs_cmd_t &cmd)
                   {
                     std::ifstream is = logging::read_log(cmd.index.value_or(1) - 1);
                     std::cout << is.rdbuf() << std::endl;
                   },
                   [](remove_specific_cmd_t &cmd)
                   {
                     inventory::remove(std::vector{inventory::resolve(cmd.image, cmd.version)});
                   },
                   [](remove_unused_cmd_t &cmd)
                   {
                     auto versions = inventory::list_versions(cmd.image);
                     for (auto &version : versions)
                     {
                       if (entity_t{cmd.image, version} == inventory::current())
                       {
                         std::cout << "Ignoring current version" << std::endl;
                         break;
                       }
                       if (version == *versions.rend())
                       {
                         std::cout << "Ignoring latest version" << std::endl;
                         break;
                       }
                       inventory::remove(std::vector{inventory::resolve(cmd.image, version)});
                     }
                   },
                   [&config](run_cmd_t &cmd)
                   {
                     auto entity = inventory::resolve(cmd.image.value_or(config.default_image_name.value_or("")), cmd.version.value_or(config.default_image_version.value_or(version_latest)));
                     if (entity.name == "")
                       throw std::runtime_error("No image name provided");

                     std::unique_ptr<logging::logger_t> logger = nullptr;
                     if (cmd.enable_logging)
                     {
                       std::cout << "Logs will be sent to file. Use 'logs' command to view them." << std::endl;
                       logger = std::make_unique<logging::file_logger>();
                     }
                     else
                       logger = std::make_unique<logging::tty_logger>();
                       
                     runner::run_mode_t mode = cmd.replace ? runner::RUN_MODE_PERMANENT : runner::RUN_MODE_TEMPORARY;
                     auto executable = cmd.executable.value_or(config.default_executable.value_or(""));
                     if (cmd.executable == "")
                       throw std::runtime_error("No executable provided");

                     std::vector<std::filesystem::path> persistent_directories = std::move(cmd.persistent_directories);
                     persistent_directories.push_back("/succ"); // TODO: make it a constant
                     if (cmd.add_default_persistent_directories)
                       persistent_directories.insert(persistent_directories.end(), config.persistent_directories.begin(), config.persistent_directories.end());

                     runner::run(*logger, mode, inventory::path(entity), runner::DEFAULT_ROOTBACK, persistent_directories, executable);
                   },
                   [](help_cmd_t &cmd)
                   {
                     std::cout << HELP_TEXTS.at(cmd.command.value_or("")) << std::endl;
                   }},
        cmd);
  }
  catch (const std::exception &e)
  {
    std::cout << "Successor failed because of " << e.what() << std::endl;
    return 1;
  }
}