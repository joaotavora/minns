# global variables used by top-level make
CXXFLAGS = -g -Wall -pedantic
MAKELIB = libtool -o $(LIBDIR)/$@ $^
MAKEBIN = $(LINK.cpp) $^ $(LDLIBS) -o $(BINDOR)/$@
