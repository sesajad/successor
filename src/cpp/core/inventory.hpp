#ifndef inventory_hpp
#define inventory_hpp

#include <string>
#include <variant>
#include <optional>
#include <vector>
#include <set>
#include <filesystem>

#include "../interfaces/system.hpp"
#include "data.hpp"

namespace inventory
{

  const std::filesystem::path INVENTORY_PATH("/succ/inv");

  std::filesystem::path inline path(entity_t entity)
  {
    return INVENTORY_PATH / entity.name / std::to_string(entity.version);
  }

  void build(entity_t entity, std::filesystem::path source)
  {
    std::string builder = "";
    if (sys::binary_exists("buildah"))
      builder = "buildah";
    if (builder == "" && sys::binary_exists("podman"))
      builder = "podman";
    if (builder == "" && sys::binary_exists("docker"))
      builder = "docker";

    if (builder == "")
      throw std::runtime_error("Cannot find any container builder. Install buildah, podman or docker");

    if (!std::filesystem::create_directories(path(entity)))
      throw std::runtime_error("Cannot create inventory directory");

    if (sys::execute(builder, {"-t",
                               entity.name + ":" + std::to_string(entity.version),
                               "-o",
                               "type=local,dest=" + path(entity).string(),
                               source.string()}) != 0)
    {
      std::filesystem::remove_all(path(entity));
      throw std::runtime_error("Cannot build image");
    }
  }

  std::vector<std::string> list_images()
  {
    std::vector<std::string> images;
    for (auto &entry : std::filesystem::directory_iterator(INVENTORY_PATH))
    {
      images.push_back(entry.path().filename().string());
    }
    return images;
  }

  std::set<int, std::less<int>> list_versions(std::string image)
  {
    for (auto &entry : std::filesystem::directory_iterator(INVENTORY_PATH))
      if (entry.path().filename().string() == image)
      {
        std::set<int> versions;
        for (auto &subentry : std::filesystem::directory_iterator(entry))
          versions.insert(std::stoi(subentry.path().filename().string()));
        return versions;
      }

    return {};
  }

  std::optional<entity_t> current()
  {
    for (auto &image : list_images())
      for (auto &version : list_versions(image))
        if (std::filesystem::equivalent(path({.name = image, .version = version}), "/"))
          return std::make_optional<entity_t>({.name = image, .version = version});

    return std::nullopt;
  }

  void remove(const std::vector<entity_t> &entities)
  {
    for (auto &entity : entities)
      if (entity.name == current()->name && entity.version == current()->version)
        throw std::runtime_error("Cannot remove current entity");
      else
        std::filesystem::remove_all(path(entity));

    for (auto &image : list_images())
      if (list_versions(image).empty())
        std::filesystem::remove_all(INVENTORY_PATH / image);
  }

  entity_t resolve(std::string image, version_t version)
  {
    int latest_version = list_versions(image).empty() ? 0 : (*list_versions(image).rend());
    if (std::holds_alternative<version_latest_t>(version))
      return {.name = image, .version = latest_version};
    else
      return {.name = image, .version = std::get<int>(version)};
  }
}

#endif