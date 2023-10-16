#ifndef mount_hpp
#define mount_hpp

#include <vector>
#include <string>
#include <mntent.h>
#include <sys/mount.h>
#include <stdexcept>

namespace mnt
{

  struct mount_t
  {
    std::string source;
    std::string target;
    std::string type;
    std::string options;
  };

  std::vector<mount_t> list(std::string path = "/proc/mounts")
  {
    std::vector<mount_t> mounts;
    FILE *fd = setmntent(path.c_str(), "r");
    struct mntent *ent;
    while (ent = getmntent(fd))
    {
      mount_t mount = {.source = ent->mnt_fsname, .target = ent->mnt_dir, .type = ent->mnt_type, .options = ent->mnt_opts};
      mounts.push_back(mount);
    }
    endmntent(fd);
    return mounts;
  }

  bool attach(const std::string &source, const std::string &target, const std::string &type, const std::string &options)
  {
    return mount(source.c_str(), target.c_str(), type.c_str(), 0, options.c_str()) == 0;
  }

  bool attach_bind(const std::string &source, const std::string &target)
  {
    return mount(source.c_str(), target.c_str(), NULL, MS_BIND, NULL) == 0;
  }

  bool move(const std::string &from, const std::string &to)
  {
    return mount(from.c_str(), to.c_str(), NULL, MS_MOVE, NULL) == 0;
  }

  bool detach(const std::string &target)
  {
    return umount(target.c_str()) == 0;
  }
}

#endif