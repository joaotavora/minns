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

tcpSocketUnit: $(LIBDIR)/libthread.a ThreadUnit.o
	$(MAKEBIN)

udpSocketUnit: $(LIBDIR)/libtcpsocket.a TcpSocketUnit.o 
	$(MAKEBIN)

threadUnit: $(LIBDIR)/libudpsocket.a UdpSocketUnit.o 
	$(MAKEBIN)

dnsResolverUnit: $(LIBDIR)/libdnsresolver.a DnsResolverUnit.cpp
	$(MAKEBIN)

clean:
	rm -rf *.o *.dSYM testThread testTcpSocket testUdpSocket


# test%: $(BASE)/src/%.o %Unit.o
# 	$(LINK.cpp) $^ $(LDLIBS) -o test$@


