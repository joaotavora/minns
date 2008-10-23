LIBS    =   # vendor/unp/lib/libunp.a
LDFLAGS =   # -lunp
LDLIBS  =   # -Lvendor/unp/lib

INC     =   # -Ivendor/unp/include

CXXFLAGS = -g -Wall -pedantic

OBJS =	TcpServer.o EchoServer.o \
	ConnectionManager.o PrethreadedConnectionManager.o \
	ConnectionHandler.o EchoHandler.o

CXXFLAGS += $(INC) 

all : minns

minns: $(OBJS) $(LIBS) 
	$(LINK.cpp) $(OBJS) $(LDLIBS) -o $@

# $(CXX) $(CXXFLAGS) -o minns $(OBJS)

TcpServer.o: TcpServer.cpp TcpServer.h ConnectionManager.h ConnectionHandler.h
ConnectionHandler.o: ConnectionHandler.h
ConnectionManager.o: ConnectionManager.h
EchoServer.o: EchoServer.cpp EchoServer.h TcpServer.h ConnectionManager.h ConnectionHandler.h
EchoHandler.o: EchoHandler.cpp EchoHandler.h ConnectionHandler.h
PrethreadedConnectionManager.o: PrethreadedConnectionManager.cpp PrethreadedConnectionManager.h ConnectionManager.h ConnectionHandler.h

clean: 
	rm -rf *.o minns *.dSYM

## third party libs, will probably get rid of this soon

# $(LIBS):
# 	cd vendor/unp/lib/lib && $(MAKE) CC=$(CXX)

# cleanvendor:
# 	cd vendor/unp/lib/lib && $(MAKE) clean