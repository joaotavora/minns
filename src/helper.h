#include <stdexcept>
#include <limits.h>

void hexdump(void *pAddressIn, long  lSize);
unsigned int strtol_helper(char c, char* arg, unsigned int const* defaults) throw (std::runtime_error);
