#ifndef log_hpp
#define log_hpp

#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <ctime>

namespace logging
{
  const std::filesystem::path LOG_PATH = "/succ/logs";

  struct logger_t
  {
    std::ostream &info;
    std::ostream &warn;
    std::ostream &err;
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

  logger_t file_logger()
  {
    if (!std::filesystem::exists(LOG_PATH))
      std::filesystem::create_directory(LOG_PATH);

    std::time_t now = std::time(nullptr);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H-%M-%S", std::localtime(&now));
    std::filesystem::path log_file_path = LOG_PATH / (std::string("log_") + timestamp + ".txt");
    std::ofstream log_file(log_file_path);

    return logger_t{log_file, log_file, log_file};
  }

  logger_t tty_logger()
  {
    return logger_t{std::cout, std::cout, std::cerr};
  }
}

#endif