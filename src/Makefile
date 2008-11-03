# Try to figure out undefined values
BASE ?= ../
CXXFLAGS ?= -g -Wall -pedantic
LDLIBS ?= -pthread

BINDIR  ?= $(CURDIR)

MAKEBIN ?= $(LINK.cpp) $^ $(LDLIBS) -o $(BINDIR)/$@

# Specific to this makefile

all : minns

minns: minns.o DnsServer.o DnsWorker.o DnsMessage.o UdpSocket.o TcpSocket.o Socket.o DnsResolver.o Thread.o helper.o
	$(MAKEBIN)

clean:
	rm -rf *.o minns echo *.dSYM

simplememtest: minns
	(dig @localhost -p 43434 bla bli)& \
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./minns

udptest: minns
	(dig @localhost -p 43434 bla +short ble +short)& ./minns

tcptest: minns
	(sleep 1;dig @localhost -p 43434 bla +short +tcp ble +short +tcp)& ./minns

tcpmemtest: minns
	(sleep 1;dig @localhost -p 43434 bla +short +tcp ble +short +tcp)&\
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./minns

optionstest: minns
	./minns -f ../test/module/simplehosts.txt -c 10asd -x -m 2 -i 1 -t -1 -u 43434 -d 1 -p 1

memtest: minns
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./minns -f simplehosts.txt -t 43434 -u 43434 -p 0

# Dependencies
DnsMessage.o: DnsMessage.cpp DnsMessage.h DnsResolver.h Thread.h
DnsResolver.o: DnsResolver.cpp DnsResolver.h
DnsServer.o: DnsServer.cpp helper.h DnsServer.h Socket.h UdpSocket.h \
  DnsMessage.h DnsResolver.h Thread.h DnsWorker.h TcpSocket.h
DnsWorker.o: DnsWorker.cpp helper.h DnsWorker.h Thread.h UdpSocket.h \
  Socket.h TcpSocket.h DnsResolver.h DnsMessage.h
EchoHandler.o: EchoHandler.cpp EchoHandler.h
EchoServer.o: EchoServer.cpp EchoServer.h Socket.h TcpSocket.h Thread.h
helper.o: helper.cpp helper.h
minns.o: minns.cpp helper.h DnsServer.h Socket.h UdpSocket.h DnsMessage.h \
  DnsResolver.h Thread.h DnsWorker.h TcpSocket.h
Socket.o: Socket.cpp Socket.h
TcpSocket.o: TcpSocket.cpp TcpSocket.h Socket.h
Thread.o: Thread.cpp Thread.h
UdpSocket.o: UdpSocket.cpp UdpSocket.h Socket.h





