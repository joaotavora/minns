# Try to figure out undefined values
BASE ?= $(CURDIR)/..

SRCDIR  ?= $(BASE)/src

# keep created bins inside current directory when built "locally"
BINDIR  ?= $(CURDIR)
MAKEBIN ?= $(LINK.cpp) $^ $(LDLIBS) -o $(BINDIR)/$@


# Specific to googletest framework
#
GTEST_DIR = ../vendor/googletest/
CPPFLAGS += -isystem $(GTEST_DIR)/include
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^


# Specific to this makefile
CXXFLAGS ?= -g -Wall -ansi -pedantic -pthread
CPPFLAGS += -I$(SRCDIR)

all: tcpSocketUnit udpSocketUnit threadUnit dnsResolverUnit

#cd $(SRCDIR) && $(MAKE) LIBDIR='$(LIBDIR)' -w -W $*.a $*.a

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp $(SRCDIR)/%.h
	cd $(SRCDIR) && $(MAKE) -w -B $*.o

tcpSocketUnit: $(SRCDIR)/TcpSocket.o $(SRCDIR)/Socket.o TcpSocketUnit.o
	$(MAKEBIN)

udpSocketUnit: $(SRCDIR)/UdpSocket.o $(SRCDIR)/Socket.o UdpSocketUnit.o
	$(MAKEBIN)

threadUnit: $(SRCDIR)/Thread.o ThreadUnit.o
	$(MAKEBIN)

dnsResolverUnit: $(SRCDIR)/DnsResolver.o DnsResolverUnit.o
	$(MAKEBIN)

clean:
	rm -rf *.o *.dSYM tcpSocketUnit udpSocketUnit threadUnit dnsResolverUnit

memtest: tcpSocketUnit
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes \
./tcpSocketUnit


# test%: $(BASE)/src/%.o %Unit.o
# 	$(LINK.cpp) $^ $(LDLIBS) -o test$@

ThreadUnit.o: ThreadUnit.cpp $(SRCDIR)/Thread.h $(SRCDIR)/Thread.cpp
TcpSocketUnit.o: TcpSocketUnit.cpp $(SRCDIR)/TcpSocket.h $(SRCDIR)/Socket.h $(SRCDIR)/TcpSocket.cpp $(SRCDIR)/Socket.cpp
UdpSocketUnit.o: UdpSocketUnit.cpp $(SRCDIR)/UdpSocket.h $(SRCDIR)/Socket.h $(SRCDIR)/UdpSocket.cpp $(SRCDIR)/Socket.cpp
DnsResolverUnit.o: DnsResolverUnit.cpp $(SRCDIR)/DnsResolver.h $(SRCDIR)/DnsResolver.cpp
