// libc includes
#include <limits.h>
#include <signal.h>

// stdl includes
#include <stdexcept>

void hexdump(void *pAddressIn, long  lSize);
unsigned int strtol_helper(char c, char* arg, unsigned int const* defaults) throw (std::runtime_error);
sighandler_t signal_helper(int signo, sighandler_t func) throw (std::runtime_error);
