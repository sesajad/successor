#ifndef config_hpp
#define config_hpp

#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <optional>
#include <variant>
#include <map>

const std::filesystem::path CONFIG_PATH = "/succ/defaults.yml";

struct config_t
{
  std::optional<std::string> default_image_name;
  std::optional<version_t> default_image_version;
  std::vector<std::string> persistent_directories;
  std::optional<std::string> default_executable;
};

struct yml_map_t;
typedef std::variant<std::string, int, std::vector<std::string>, yml_map_t> yml_value_t;
struct yml_map_t : public std::map<std::string, yml_value_t>
{
};

std::string trim(std::string str)
{
  auto whitespaces = " \t\n\r\f\v";
  str.erase(0, str.find_first_not_of(whitespaces));
  str.erase(str.find_last_not_of(whitespaces) + 1);
  return str;
}

config_t load_config(std::filesystem::path path = CONFIG_PATH)
{
  config_t config;

  if (!std::filesystem::exists(path))
    return config;

  std::ifstream file(path);
  std::string line;
  while (std::getline(file, line))
  {

    // removing comments
    if (line.find('#') != std::string::npos)
    {
      line = line.substr(0, line.find('#'));
    }

    if (line.find("image") == 0)
      config.default_image_name = trim(line.substr(line.find(':') + 1));

    if (line.find("version") == 0)
      if (line.find("latest") != std::string::npos)
        config.default_image_version = version_latest;
      else
        config.default_image_version = std::stoi(trim(line.substr(line.find(':') + 1)));

    if (line.find("executable") == 0)
      config.default_executable = trim(line.substr(line.find(':') + 1));

    if (line.find("persistent_dirs") == 0)
      continue;

    if (trim(line).find('-') == 0)
      config.persistent_directories.push_back(trim(trim(line).substr(1)));
  }

  return config;
}

#endif