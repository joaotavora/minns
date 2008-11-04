# global variables used by top-level make
export CXXFLAGS = -O3
export LDLIBS = -pthread

MAKEBIN = $(LINK.cpp) $^ $(LDLIBS) -o $(BINDOR)/$@
