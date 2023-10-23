#include "../interfaces/log.hpp"

BOOST_AUTO_TEST_CASE(test_file_log)
{
  logging::file_logger logger;
  logger.info() << "Hello, world!" << std::endl;
  logger.warn() << "Hello, world!" << std::endl;
  logger.error() << "Hello, world!" << std::endl;
}

BOOST_AUTO_TEST_CASE(test_tty_log)
{
  logging::tty_logger logger;
  logger.info() << "Hello, world!" << std::endl;
  logger.warn() << "Hello, world!" << std::endl;
  logger.error() << "Hello, world!" << std::endl;
}