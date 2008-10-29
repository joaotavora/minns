LDFLAGS =   # -lunp
LDLIBS  =   # -L$(BASE)/vendor/unp/lib
INC     =   # -I$(BASE)/vendor/unp/include

CXXFLAGS += $(INC)

LIBDIR  ?= $(CURDIR)
BINDIR  ?= $(CURDIR)

MAKELIB ?= libtool -o $(LIBDIR)/$@ $^
MAKEBIN ?= $(LINK.cpp) $^ $(LDLIBS) -o $(BINDOR)/$@

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

clean: 
	rm -rf *.o $(BINDIR)/minns $(BINDIR)/echo *.dSYM

cleanlibs:
	rm -rf $(LIBDIR)/libthread.a $(LIBDIR)/libtcpsocket.a $(LIBDIR)/libudpsocket.a


# Dependencies

EchoServer.o: EchoServer.cpp EchoServer.h TcpSocket.h Socket.h Thread.h
DnsServer.o: DnsServer.cpp DnsServer.h UdpSocket.h Socket.h DnsResolver.h

Socket.o: Socket.cpp Socket.h
TcpSocket.o: TcpSocket.cpp TcpSocket.h Socket.h
UdpSocket.o: UdpSocket.cpp UdpSocket.h Socket.h
Thread.o: Thread.cpp Thread.h
DnsResolver.o: DnsResolver.cpp DnsResolver.h


