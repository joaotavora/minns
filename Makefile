CXXFLAGS = -g -Wall -pedantic

INC  =  -Ivendor/unp/lib
OBJS =	TcpServer.o DnsServer.o \
	ConnectionManager.o PrethreadedConnectionManager.o \
	ConnectionHandler.o EchoHandler.o

CXXFLAGS += $(INC) 

all : minns

minns: $(OBJS)
	$(CXX) $(CXXFLAGS) -o minns $(OBJS)

TcpServer.o: TcpServer.cpp TcpServer.h ConnectionManager.h ConnectionHandler.h
DnsServer.o: DnsServer.cpp DnsServer.h TcpServer.h ConnectionManager.h ConnectionHandler.h
EchoHandler.o: EchoHandler.cpp EchoHandler.h ConnectionHandler.h
PrethreadedConnectionManager.o: PrethreadedConnectionManager.cpp PrethreadedConnectionManager.h ConnectionManager.h ConnectionHandler.h

clean:
	rm -rf *.o minns *.dSYM