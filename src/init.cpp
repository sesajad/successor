#include <functional>
#include <filesystem>

#include <unistd.h>
#include <syscall.h>
#include <sys/mount.h>

#include "records.hpp"
#include "logging.hpp"
#include "cli.hpp"
#include "migration.hpp"

int main(int argc, char **argv)
{
  std::cout << "Successor is starting..." << std::endl;

  std::cout << "Stage 1: Initializing..." << std::endl;
  vector<function<void()>> deferred_actions;
  function<void()> rollback = [&deferred_actions]()
  {
    for (auto i = deferred_actions.rbegin(); i != deferred_actions.rend(); i++)
      (*i)();
    deferred_actions.clear();
  };

  Arguments state = Arguments{.mode = Arguments::NORMAL};
  try
  {
    state = parse_args(argc, argv);
  }
  catch (runtime_error &e)
  {
    std::cout << e.what() << std::endl;
    print_help();
    return 1;
  }

  logger_t logger(state.log_path);
  logger.info << "Loading migration plan..." << logger.endl;
  ifstream record_file(state.plan_path);
  deferred_actions.push_back([&record_file]()
                             { record_file.close(); });
  if (!record_file.good())
  {
    logger.error << "Error: Cannot read directories.yml." << logger.endl;
    rollback();
    exit(EXIT_FAILURE);
  }

  record_t records;
  try
  {
    records = from_yaml(record_file);
  }
  catch (runtime_error &e)
  {
    logger.error << e.what() << logger.endl;
    rollback();
    exit(EXIT_FAILURE);
  }

  logger.info << "Parsed migration plan:" << logger.endl;
  to_yaml(std::cout, records);

  // logger.info << "Stage 2: Preparing temproot..." << logger.endl;
  
  // string temproot_path_rt_root = state.temproot_path;
  // string old_path_rt_root = temproot_path_rt_root + state.oldroot_path_rt_temproot;
  // string new_path_rt_root = old_path_rt_root + state.newroot_path;
  // logger.info << "temproot path (relative to current root): " << temproot_path_rt_root << logger.endl;
  // logger.info << "old path (relative to current root): " << old_path_rt_root << logger.endl;
  // logger.info << "new path (relative to current root): " << new_path_rt_root << logger.endl;

  // logger.info << "Creating directory for temproot" << logger.endl;
  // if (!filesystem::create_directory(temproot_path_rt_root)) {
  //   logger.error << "Warning: Cannot create directory for temproot. Probably it exists" << logger.endl;
  // }

  // logger.info << "Mounting temproot" << logger.endl;
  // if (mount(NULL, temproot_path_rt_root.c_str(), "tmpfs", 0, NULL) != 0)
  // {
  //   logger.error << "Error: Cannot mount the temporarily filesystem." << logger.endl;
  //   rollback();
  //   exit(EXIT_FAILURE);
  // }
  // deferred_actions.push_back([&logger, &temproot_path_rt_root]()
  //                            {
  //   if (umount(temproot_path_rt_root.c_str()) != 0)
  //   {
  //     logger.error << "Ignored Error: Cannot unmount temproot." << logger.endl;
  //   }
  // });

  // logger.info << "Creating directory for new root" << logger.endl;
  // if (!filesystem::create_directory(old_path_rt_root)) {
  //   logger.error << "Warning: Cannot create directory for old root. Probably it exists" << logger.endl;
  // }

  // logger.info << "Pivoting into temproot" << logger.endl;
  // if (syscall(SYS_pivot_root, temproot_path_rt_root.c_str(), old_path_rt_root.c_str()) != 0)
  // {
  //   logger.error << "Error: Cannot pivot root." << logger.endl;
  //   rollback();
  //   exit(EXIT_FAILURE);
  // }

  // logger.info << "Changing directory to temproot" << logger.endl;
  // if (chdir("/") != 0)
  // {
  //   logger.error << "Error: Cannot change directory." << logger.endl;
  //   rollback();
  //   exit(EXIT_FAILURE);
  // }

  // if (mount(NULL, state.oldroot_path_rt_temproot.c_str(), NULL, MS_REMOUNT, "rw") != 0)
  // {
  //   logger.error << "Error: Cannot remount old root as rw." << logger.endl;
  //   rollback();
  //   exit(EXIT_FAILURE);
  // }

  // std::string old_path_rt_temproot = state.oldroot_path_rt_temproot;
  // std::string new_path_rt_temproot = state.oldroot_path_rt_temproot + state.newroot_path;
  // std::string temproot_path_rt_temproot = state.oldroot_path_rt_temproot + temproot_path_rt_root;
  // logger.info << "old path (relative to temproot): " << old_path_rt_temproot << logger.endl;
  // logger.info << "new path (relative to temproot): " << new_path_rt_temproot << logger.endl;
  // logger.info << "temproot path (relative to temproot): " << temproot_path_rt_temproot << logger.endl;

  // deferred_actions.push_back([&logger, &old_path_rt_temproot, &temproot_path_rt_temproot]() {
  //   if (syscall(SYS_pivot_root, old_path_rt_temproot.c_str(), temproot_path_rt_temproot.c_str()) != 0) {
  //     logger.error << "Ignored Error: Cannot pivot back to root." << logger.endl;
  //   }
  //   if (chdir("/") != 0) {
  //     logger.error << "Ignored Error: Cannot change directory." << logger.endl;
  //   } });

  auto old_path_rt_temproot = "/";
  auto new_path_rt_temproot = "/succ/new";

  logger.info << "Stage 3: Migrating..." << logger.endl;
  logger.info << "Testing migration..." << logger.endl;
  if (!migrate(logger, filesystem::path(old_path_rt_temproot), filesystem::path(new_path_rt_temproot), records, true))
  {
    logger.error << "Error: dry-run failed." << logger.endl;
    rollback();
    exit(EXIT_FAILURE);
  }

  logger.info << "Starting migration..." << logger.endl;
  try {
  if (state.mode == Arguments::NORMAL)
    migrate(logger, filesystem::path(old_path_rt_temproot), filesystem::path(new_path_rt_temproot), records);
  } catch (const filesystem::filesystem_error &e) {
    logger.error << "Fatal Error: " << e.what() << logger.endl;
    rollback();
    exit(EXIT_FAILURE);
  } catch (const runtime_error &e) {
    logger.error << "Fatal Error: " << e.what() << logger.endl;
    rollback();
    exit(EXIT_FAILURE);
  }
  logger.info << "Migration completed." << logger.endl;

  rollback();

  if (state.next_executable_path)
    if (execl(state.next_executable_path->c_str(), state.next_executable_path->c_str(), NULL) != 0)
    {
      logger.error << "Error: Cannot execute next command." << logger.endl;
      exit(EXIT_FAILURE);
    }

  return 0;
}