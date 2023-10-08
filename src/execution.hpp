#ifndef execution_hpp
#define execution_hpp

#include <unistd.h>
#include <iostream>

using namespace std;

bool run_subprocess(string command)
{
  FILE *res = popen(command.c_str(), "r");
  if (res == NULL)
  {
    cerr << "Error: Cannot open subprocess." << endl;
    return false;
  }

  int status = pclose(res);
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
  {
    cerr << "Error: Subprocess failed." << endl;
    return false;
  }

  return true;
}

inline void panic_to_init()
{
  cout << "Resuming init process..." << endl;
  execl("/sbin/init", "/sbin/init", NULL);
}

inline void panic_to_shell()
{
  cout << "Entering rescue mode..." << endl;
  cout << "Run /sbin/init to start the operating system." << endl;
  execl("/bin/sh", "/bin/sh", NULL);
}

inline void panic()
{
  cout << "Process terminated." << endl;
  exit(EXIT_FAILURE);
}


#endif