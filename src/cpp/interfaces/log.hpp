#ifndef log_hpp
#define log_hpp

#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <ctime>

namespace logging
{
  const std::filesystem::path LOG_PATH = "/succ/log";

  class logger_t
  {
  public:
    virtual std::ostream &info() = 0;
    virtual std::ostream &warn() = 0;
    virtual std::ostream &error() = 0;
  };

  std::ifstream read_log(int index)
  {
    std::vector<std::filesystem::path> log_files;
    for (const auto &entry : std::filesystem::directory_iterator(LOG_PATH))
      log_files.push_back(entry.path());

    std::sort(log_files.begin(), log_files.end());

    if (index >= log_files.size())
      throw std::runtime_error("Invalid log index");

    std::ifstream stream(log_files[log_files.size() - index - 1]);
    return std::move(stream);
  }

  class file_logger : public logger_t
  {
    std::ofstream log_file;

  public:
    file_logger()
    {
      std::time_t now = std::time(nullptr);
      char timestamp[20];
      std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H-%M-%S", std::localtime(&now));
      std::filesystem::path log_file_path = LOG_PATH / (std::string("log_") + timestamp + ".txt");
      log_file.open(log_file_path);
      if (!log_file.is_open())
        throw std::runtime_error("Cannot open log file");
    }

    std::ostream &info() override
    {
      return log_file;
    }
    std::ostream &warn() override
    {
      return log_file;
    }
    std::ostream &error() override
    {
      return log_file;
    }
  };

  class tty_logger : public logger_t
  {
  public:
    std::ostream &info() override
    {
      return std::cout;
    }
    std::ostream &warn() override
    {
      return std::cout;
    }
    std::ostream &error() override
    {
      return std::cout;
    }
  };
}

#endif