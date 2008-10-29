# Try to figure out undefined values
BASE ?= ../
CXXFLAGS ?= -g -Wall -pedantic

LIBDIR  ?= $(CURDIR)
BINDIR  ?= $(CURDIR)

MAKELIB ?= libtool -o $(LIBDIR)/$@ $^
MAKEBIN ?= $(LINK.cpp) $^ $(LDLIBS) -o $(BINDOR)/$@

# Specific to this makefile

all : minns

minns: DnsServer.o UdpSocket.o Socket.o DnsResolver.o
	$(MAKEBIN)

echo: EchoServer.o TcpSocket.o Socket.o Thread.o
	$(MAKEBIN)

libs: libthread.a libtcpsocket.a libudpsocket.a

libthread.a: Thread.o
	$(MAKELIB)
libtcpsocket.a: TcpSocket.o Socket.o
	$(MAKELIB)
libudpsocket.a: UdpSocket.o Socket.o
	$(MAKELIB)

clean: cleanlibs 
	rm -rf *.o minns echo *.dSYM

cleanlibs:
	rm -rf libthread.a libtcpsocket.a libudpsocket.a

# Dependencies

EchoServer.o: EchoServer.cpp EchoServer.h TcpSocket.h Socket.h Thread.h
DnsServer.o: DnsServer.cpp DnsServer.h UdpSocket.h Socket.h DnsResolver.h

Socket.o: Socket.cpp Socket.h
TcpSocket.o: TcpSocket.cpp TcpSocket.h Socket.h
UdpSocket.o: UdpSocket.cpp UdpSocket.h Socket.h
Thread.o: Thread.cpp Thread.h
DnsResolver.o: DnsResolver.cpp DnsResolver.h


