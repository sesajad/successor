#include "../interfaces/system.hpp"

BOOST_AUTO_TEST_CASE(test_execute)
{
  BOOST_CHECK_EQUAL(sys::execute("echo", {"hello"}, false, true), 0);
  BOOST_CHECK_NE(sys::execute("cat", {"/nonexistent"}, false, true), 0);
}

BOOST_AUTO_TEST_CASE(test_binary_exists)
{
  BOOST_CHECK(sys::binary_exists("cat"));
  BOOST_CHECK(!sys::binary_exists("nonexistent"));
}
