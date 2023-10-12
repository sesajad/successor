#ifndef mount_hpp
#define mount_hpp

#include <vector>
#include <string>
#include <mntent.h>
#include <sys/mount.h>
#include <stdexcept>

struct mount_t {
  std::string source;
  std::string target;
  std::string type;
  std::string options;
};

std::vector<mount_t> get_mounts() {
  std::vector<mount_t> mounts;
  FILE* fd = setmntent("/proc/mounts", "r");
  struct mntent *ent;
  while (ent = getmntent(fd)) {
    mount_t mount = {.source = ent->mnt_fsname, .target = ent->mnt_dir, .type = ent->mnt_type, .options = ent->mnt_opts};
    mounts.push_back(mount);
  }
  endmntent(fd);
  return mounts;
}

void add_mount(const std::string &source, const std::string &target, const std::string &type, const std::string &options) {
  if (mount(source.c_str(), target.c_str(), type.c_str(), 0, options.c_str()) != 0)
    throw std::runtime_error("Error: Cannot mount " + source + " to " + target + ".");
}

void del_mount(const std::string &target) {
  if (umount(target.c_str()) != 0)
    throw std::runtime_error("Error: Cannot unmount " + target + ".");
}

#endif