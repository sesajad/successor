#ifndef logging_hpp
#define logging_hpp

#include <iostream>
#include <fstream>
#include <optional>
#include <unistd.h>

using namespace std;

struct multiostream : ostream {
  ostream &os1, &os2;
  multiostream(ostream &os1, ostream &os2) : os1(os1), os2(os2) {}
  template <typename T>
  auto& operator<<(const T &value)
  {
    os1 << value;
    os2 << value;
    os1.flush();
    os2.flush();
    usleep(500000);
    return *this;
  }
};
 

struct logger_t {
public:
  ofstream log_file;
  logger_t(string path)
  : log_file(path), info(cout, log_file), error(cerr, log_file)
  { }

  ~logger_t()
  {
    log_file.close();
  }

  multiostream info;
  multiostream error;

  const char* endl = "\n";
};

#endif