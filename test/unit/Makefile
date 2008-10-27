-include $(BASE)/Makefile.inc

BASE ?= ../../
CXXFLAGS ?= -g -Wall -ansi -pedantic

INC += -I$(BASE)src

CXXFLAGS += $(INC)

TESTS = testTcpSocket

all: $(TESTS)

testTcpSocket: $(BASE)/src/TcpSocket.o $(BASE)/src/TcpSocket.h TcpSocketUnit.o 
	$(LINK.cpp) $^ $(LDLIBS) -o $@


# test%: $(BASE)/src/%.o %Unit.o
# 	$(LINK.cpp) $^ $(LDLIBS) -o test$@


