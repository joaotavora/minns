LIBS    =   # $(BASE)/vendor/unp/lib/libunp.a
LDFLAGS =   # -lunp
LDLIBS  =   # -L$(BASE)/vendor/unp/lib
INC     =   # -I$(BASE)/vendor/unp/include

CXXFLAGS += $(INC) 

all : minns

minns: DnsServer.o UdpSocket.o Socket.o DnsResolver.o $(LIBS)
	$(LINK.cpp) $^ $(LDLIBS) -o $@

echo: EchoServer.o TcpSocket.o Socket.o Thread.o $(LIBS)
	$(LINK.cpp) $^ $(LDLIBS) -o $@

EchoServer.o: EchoServer.cpp EchoServer.h TcpSocket.h Socket.h Thread.h
DnsServer.o: DnsServer.cpp DnsServer.h UdpSocket.h Socket.h DnsResolver.h

Socket.o: Socket.cpp Socket.h
TcpSocket.o: TcpSocket.cpp TcpSocket.h Socket.h
UdpSocket.o: UdpSocket.cpp UdpSocket.h Socket.h
Thread.o: Thread.cpp Thread.h
DnsResolver.o: DnsResolver.cpp DnsResolver.h

clean: 
	rm -rf *.o minns echo *.dSYM

## third party libs, will probably get rid of this soon

# $(LIBS):
# 	cd vendor/unp/lib/lib && $(MAKE) CC=$(CXX)

# cleanvendor:
# 	cd vendor/unp/lib/lib && $(MAKE) clean