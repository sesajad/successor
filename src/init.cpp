#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>
#include <vector>
#include <set>
#include <algorithm>
#include <functional>

#include "execution.hpp"
#include "records.hpp"


using namespace std;

const string MAIN_DIR = "/succ/";

bool migrate(filesystem::path src,
             filesystem::path dst,
             const record_t &record,
             bool dry_run = false)
{
  switch (record.action)
  {
  case record_t::WIPE:
  {
    if (!dry_run)
    {
      if (filesystem::exists(src))
        filesystem::remove_all(src);

      if (filesystem::exists(dst))
        filesystem::remove_all(dst);
    }
    else
      cout << "Wiping " << src << endl;
    return true;
  }
  case record_t::SOURCE:
  {
    if (!dry_run)
    {
      if (filesystem::exists(dst))
        filesystem::remove_all(dst);
    }
    else
      cout << "Keeping " << src << endl;
    return true;
  }
  case record_t::DESTINATION:
  {
    if (!dry_run)
    {
      if (filesystem::exists(src))
        filesystem::remove_all(src);

      if (filesystem::exists(dst))
        filesystem::rename(dst, src);
    }
    else
      cout << "Moving " << dst << " to " << src << endl;
    return true;
  }
  case record_t::MERGE:
  {
    if (dry_run)
      cout << "Merging " << src << " and " << dst << endl;
    set<string> children;
    for (const auto &child : filesystem::directory_iterator(src))
      children.insert(child.path().filename());

    for (const auto &child : filesystem::directory_iterator(dst))
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
          cout << "Error: Cannot find child record for " << child << endl;
        return false;
      }
      else
      {
        if (!migrate(src / child, dst / child, *result, dry_run))
          return false;
      }
    }
    return true;
  }
  default:
    return false;
  }
}

struct Arguments
{
  enum 
  {
    INIT,
    HELP,
    TEST
  } mode;
};

Arguments parse_args(int argc, char **argv)
{
  if (argc == 0)
    return Arguments{.mode = Arguments::INIT};
  else if (argc == 1)
  {
    if (string(argv[0]) == "--help" || string(argv[0]) == "-h")
      return Arguments{.mode = Arguments::HELP};
    else if (string(argv[0]) == "--test" || string(argv[0]) == "-t")
      return Arguments{.mode = Arguments::TEST};
    else
      throw runtime_error("Error: Invalid argument.");
  }
  else
    throw runtime_error("Error: Too many arguments.");
}

void print_help()
{
  cout << "Usage: succ [OPTION]" << endl;
  cout << "  -h, --help     Display this help and exit." << endl;
  cout << "  -t, --test     Run in test mode." << endl;
}

int main(int argc, char **argv)
{
  cout << "Successor is starting..." << endl;
  Arguments state;
  try
  {
    state = parse_args(argc, argv);
  }
  catch (runtime_error &e)
  {
    cerr << e.what() << endl;
    print_help();
    return 1;
  }

  if (state.mode == Arguments::INIT && getpid() != 1)
  {
    cerr << "The process is not running as PID 1, run it in test mode." << endl;
    panic();
  }

  function<void()> error_handler = []() { panic(); };

  if (state.mode == Arguments::INIT)
    error_handler = panic_to_init;

  if (!filesystem::exists(MAIN_DIR + "migration-flag"))
  {
    cout << "Migration flag not found. Starting the operating system..." << endl;
    error_handler();
  }

  if (state.mode == Arguments::INIT)
    filesystem::remove(MAIN_DIR + "migration-flag");

  if (state.mode == Arguments::INIT)
    error_handler = panic_to_shell;

  cout << "Mounting the filesystem..." << endl;
  if (!run_subprocess("sh /succ/mount.sh"))
  {
    cerr << "Error: Cannot mount the filesystem." << endl;
    error_handler();
  }

  cout << "Loading migration plan..." << endl;
  ifstream record_file(MAIN_DIR + "directories.yml");
  if (!record_file.good())
  {
    cerr << "Error: Cannot read directories.yml." << endl;
    error_handler();
  }

  record_t records;
  try
  {
    records = from_yaml(record_file);
  }
  catch (runtime_error &e)
  {
    cerr << e.what() << endl;
    error_handler();
  }

  cout << "Testing migration..." << endl;
  if (!migrate(filesystem::path(MAIN_DIR + "new/"), filesystem::path(MAIN_DIR + "old/"), records, true))
  {
    cerr << "Error: dry-run failed." << endl;
    error_handler();
  }

  cout << "Starting migration..." << endl;
  if (state.mode == Arguments::INIT)
    migrate(filesystem::path(MAIN_DIR + "new/"), filesystem::path(MAIN_DIR + "old/"), records);

  cout << "Migration completed." << endl;
  if (state.mode == Arguments::INIT)
    panic_to_init();

  return 0;
}