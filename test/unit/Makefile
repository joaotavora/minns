# Try to figure out undefined values
BASE ?= $(CURDIR)/../..

SRCDIR  ?= $(BASE)/src

# keep created bins inside current directory when built "locally"
BINDIR  ?= $(CURDIR)

LIBDIR  ?= $(BASE)/src

MAKELIB ?= libtool -o $(LIBDIR)/$@ $^
MAKEBIN ?= $(LINK.cpp) $^ $(LDLIBS) -o $(BINDIR)/$@

# Specific to this makefile

CXXFLAGS ?= -g -Wall -ansi -pedantic

INC += -I$(SRCDIR)
CXXFLAGS += $(INC)

all: tcpSocketUnit udpSocketUnit threadUnit dnsResolverUnit 

#cd $(SRCDIR) && $(MAKE) LIBDIR='$(LIBDIR)' -w -W $*.a $*.a

$(LIBDIR)/%.a:
	cd $(SRCDIR) && $(MAKE) LIBDIR='$(LIBDIR)' -w $*.a

tcpSocketUnit: $(LIBDIR)/libtcpsocket.a TcpSocketUnit.o
	$(MAKEBIN)

udpSocketUnit: $(LIBDIR)/libudpsocket.a UdpSocketUnit.o
	$(MAKEBIN)

threadUnit: $(LIBDIR)/libthread.a ThreadUnit.o
	$(MAKEBIN)

dnsResolverUnit: $(LIBDIR)/libdnsresolver.a DnsResolverUnit.o
	$(MAKEBIN)

clean:
	rm -rf *.o *.dSYM tcpSocketUnit udpSocketUnit threadUnit dnsResolverUnit

# test%: $(BASE)/src/%.o %Unit.o
# 	$(LINK.cpp) $^ $(LDLIBS) -o test$@

ThreadUnit.o: ThreadUnit.cpp $(SRCDIR)/Thread.h 
TcpSocketUnit.o: TcpSocketUnit.cpp $(SRCDIR)/TcpSocket.h $(SRCDIR)/Socket.h
UdpSocketUnit.o: UdpSocketUnit.cpp $(SRCDIR)/UdpSocket.h $(SRCDIR)/Socket.h
DnsResolverUnit.o: DnsResolverUnit.cpp $(SRCDIR)/DnsResolver.h


