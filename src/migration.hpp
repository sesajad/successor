#ifndef migration_hpp
#define migration_hpp

#include <filesystem>
#include <algorithm>
#include <set>

#include "logging.hpp"
#include "records.hpp"

using namespace std;

bool migrate(logger_t& logger, 
             filesystem::path old_path,
             filesystem::path dst_path,
             const record_t &record,
             bool dry_run = false)
{
  switch (record.action)
  {
  case record_t::WIPE:
  {
    if (!dry_run)
    {
      if (filesystem::exists(old_path))
        filesystem::remove_all(old_path);

      if (filesystem::exists(dst_path))
        filesystem::remove_all(dst_path);
    }
    else
      logger.info << "Wiping " << old_path << logger.endl;
    return true;
  }
  case record_t::KEEP:
  {
    if (!dry_run)
    {
      if (filesystem::exists(dst_path))
        filesystem::remove_all(dst_path);
    }
    else
      logger.info << "Keeping " << old_path << logger.endl;
    return true;
  }
  case record_t::REPLACE:
  {
    if (!dry_run)
    {
      if (filesystem::exists(old_path))
        filesystem::remove_all(old_path);

      if (filesystem::exists(dst_path))
        filesystem::rename(dst_path, old_path);
    }
    else
      logger.info << "Moving " << dst_path << " to " << old_path << logger.endl;
    return true;
  }
  case record_t::MERGE:
  {
    if (dry_run)
      logger.info << "Merging " << old_path << " and " << dst_path << logger.endl;
    set<string> children;
    if (filesystem::exists(old_path))
      for (const auto &child : filesystem::directory_iterator(old_path))
        children.insert(child.path().filename());

    if (filesystem::exists(dst_path))
      for (const auto &child : filesystem::directory_iterator(dst_path))
        children.insert(child.path().filename());

    for (const auto &child : children)
    {
      auto result = find_if(record.children->begin(), record.children->end(), [child](const record_t &record)
                            { return record.name == child; });

      if (result == record.children->end())
      {
        if (!dry_run)
          throw runtime_error("Error: Cannot find child record.");
        else
          logger.error << "Error: Cannot find child record for " << child << logger.endl;
        return false;
      }
      else
      {
        if (!migrate(logger, old_path / child, dst_path / child, *result, dry_run))
          return false;
      }
    }
    return true;
  }
  default:
    return false;
  }
}


#endif