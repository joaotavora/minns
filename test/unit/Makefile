# Try to figure out undefined values
BASE ?= $(CURDIR)/../..

SRCDIR  ?= $(BASE)/src

# keep created bins inside current directory when built "locally"
BINDIR  ?= $(CURDIR)
MAKEBIN ?= $(LINK.cpp) $^ $(LDLIBS) -o $(BINDIR)/$@

# Specific to this makefile

CXXFLAGS ?= -g -Wall -ansi -pedantic -pthread

INC += -I$(SRCDIR)
CXXFLAGS += $(INC)

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


