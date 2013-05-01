// libstdc++ includes
#include <iostream>
#include <fstream>
#include <sstream>

// project includes
#include "DnsResolver.h"
#include "gtest/gtest.h"

// usings
using namespace std;


TEST(SucessfulResolution, SimpleNames) {

DnsResolver resolver("test/simplehosts.txt", 10, 10, 2);
EXPECT_NO_THROW(string result(resolver.resolve_to_string("bla")));
EXPECT_EQ (1,1);
}
