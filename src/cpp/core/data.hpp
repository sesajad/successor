#ifndef data_hpp
#define data_hpp

#include <string>
#include <set>
#include <variant>
#include <regex>


struct version_latest_t {};
const version_latest_t version_latest;
typedef std::variant<version_latest_t, int> version_t;

struct entity_t
{
  std::string name;
  int version;

  bool operator==(const entity_t &other) const
  {
    return name == other.name && version == other.version;
  }

  bool operator!=(const entity_t &other) const
  {
    return !(*this == other);
  }
};

const std::regex IMAGE_NAME_REGEX("^[a-zA-Z0-9_-]+$");

#endif