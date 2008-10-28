-include $(BASE)/Makefile.inc

BASE ?= ../../
CXXFLAGS ?= -g -Wall -ansi -pedantic

INC += -I$(BASE)src

CXXFLAGS += $(INC)

TESTS = testTcpSocket

all: $(TESTS)

testTcpSocket: $(BASE)/src/TcpSocket.o $(BASE)/src/TcpSocket.h $(BASE)/src/Socket.o $(BASE)/src/Socket.h TcpSocketUnit.o 
	$(LINK.cpp) $^ $(LDLIBS) -o $@

testUdpSocket: $(BASE)/src/UdpSocket.o $(BASE)/src/UdpSocket.h $(BASE)/src/Socket.o $(BASE)/src/Socket.h UdpSocketUnit.o 
	$(LINK.cpp) $^ $(LDLIBS) -o $@


# test%: $(BASE)/src/%.o %Unit.o
# 	$(LINK.cpp) $^ $(LDLIBS) -o test$@


