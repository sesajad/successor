#ifndef system_hpp
#define system_hpp

#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <mntent.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/wait.h>

namespace sys
{
  class system_error : public std::runtime_error
  {
  public:
    system_error(std::string message) : std::runtime_error(message) {}
  };

  void new_mount_namespace()
  {
    if (unshare(CLONE_NEWNS) != 0)
      throw system_error("Cannot create new mount namespace. Error code: " + std::string(std::strerror(errno)));
  }

  void pivot_root(std::string new_root, std::string put_old)
  {
    if (syscall(SYS_pivot_root, new_root.c_str(), put_old.c_str()) != 0)
      throw system_error("Cannot pivot root. Error code: " + std::string(std::strerror(errno)));
  }

  // add args
  int execute(std::string executable, std::vector<std::string> args = {}, bool replace = false)
  {
    char *arglist[args.size() + 1];
    arglist[0] = (char *)(executable.c_str());
    for (int i = 0; i < args.size(); i++)
      arglist[i + 1] = (char *)(args[i].c_str());

    pid_t pid = 0;
    if (!replace)
    {
      pid = fork();
      if (pid == -1)
        throw system_error("Cannot fork process. Error code: " + std::string(std::strerror(errno)));
    }
    if (pid == 0)
    {
      if (execv(executable.c_str(), arglist) == -1)
        throw system_error("Cannot execute command. Error code: " + std::string(std::strerror(errno)));
      return 0; // unreachable
    }
    else
    {
      int status;
      if (waitpid(pid, &status, 0) == -1)
        throw system_error("Cannot wait for forked process. Error code: " + std::string(std::strerror(errno)));
      return status;
    }
  }

  bool binary_exists(std::string name)
  {
    FILE *f = popen(("which " + name).c_str(), "r");
    if (f == NULL)
      throw system_error("Cannot use which. Error code: " + std::string(std::strerror(errno)));
    int res = pclose(f);
    if (res == -1)
      throw system_error("Cannot wait for which. Error code: " + std::string(std::strerror(errno)));
    return res == 0;
  }

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

    void attach(const std::string &source, const std::string &target, const std::string &type, const std::string &options)
    {
      if (mount(source.c_str(), target.c_str(), type.c_str(), 0, options.c_str()) != 0)
        throw system_error("Cannot attach mountpoint. Error code: " + std::string(std::strerror(errno)));
    }

    void bind(const std::string &source, const std::string &target)
    {
      if (mount(source.c_str(), target.c_str(), NULL, MS_BIND, NULL) != 0)
        throw system_error("Cannot bind mountpoint. Error code: " + std::string(std::strerror(errno)));
    }

    void move(const std::string &from, const std::string &to)
    {
      if (mount(from.c_str(), to.c_str(), NULL, MS_MOVE, NULL) != 0)
        throw system_error("Cannot move mountpoint. Error code: " + std::string(std::strerror(errno)));
    }

    void detach(const std::string &target)
    {
      if (umount(target.c_str()) != 0)
        throw system_error("Cannot detach mountpoint. Error code: " + std::string(std::strerror(errno)));
    }
  }

}

#endif