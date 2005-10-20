#include "slow.h"

#include <boost/test/unit_test.hpp>

using namespace ::boost::unit_test;
namespace slow_test {
const char* fname = "slow.test-delete-me";

void cleanup() {
	slow::remove( fname );
}

}
