# Try to figure out undefined values
BASE ?= ../
CXXFLAGS ?= -g -Wall -pedantic

LIBDIR  ?= $(CURDIR)
BINDIR  ?= $(CURDIR)

MAKELIB ?= libtool -o $(LIBDIR)/$@ $^
MAKEBIN ?= $(LINK.cpp) $^ $(LDLIBS) -o $(BINDIR)/$@

# Specific to this makefile

all : minns

minns: DnsServer.o DnsWorker.o DnsMessage.o UdpSocket.o Socket.o DnsResolver.o
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
libdnsresolver.a: DnsResolver.o
	$(MAKELIB)

clean: cleanlibs 
	rm -rf *.o minns echo *.dSYM

cleanlibs:
	rm -rf libthread.a libtcpsocket.a libudpsocket.a libdnsresolver.a

simplememtest: minns
	(dig @localhost -4 -p 43434 bla +short ble +short)& \
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./minns

# Dependencies

DnsMessage.o: DnsMessage.cpp DnsMessage.h DnsResolver.h
DnsResolver.o: DnsResolver.cpp DnsResolver.h
DnsServer.o: DnsServer.cpp DnsServer.h Socket.h UdpSocket.h DnsMessage.h \
  DnsResolver.h DnsWorker.h Thread.h TcpSocket.h
DnsWorker.o: DnsWorker.cpp DnsWorker.h Thread.h UdpSocket.h Socket.h \
  TcpSocket.h
EchoHandler.o: EchoHandler.cpp EchoHandler.h
EchoServer.o: EchoServer.cpp EchoServer.h Socket.h TcpSocket.h Thread.h
Socket.o: Socket.cpp Socket.h
TcpSocket.o: TcpSocket.cpp TcpSocket.h Socket.h
Thread.o: Thread.cpp Thread.h
UdpSocket.o: UdpSocket.cpp UdpSocket.h Socket.h



