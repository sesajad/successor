#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <string>
#include <unistd.h>
#include <sys/syscall.h>

namespace sys {
  bool pivot_root(std::string new_root, std::string put_old) {
    return syscall(SYS_pivot_root, new_root.c_str(), put_old.c_str()) == 0;
  }

  void execute(std::string path) {
    execl(path.c_str(), path.c_str(), NULL);
  }

  bool change_dir(std::string path) {
    return chdir(path.c_str()) == 0;
  }

  int get_pid() {
    return getpid();
  }
}

#endif