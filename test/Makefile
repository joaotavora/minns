# Try to figure out undefined values
BASE ?= $(CURDIR)/..

SRCDIR  ?= $(BASE)/src

include Makefile.googletest

# Specific to this makefile
CXX=clang++
CXXFLAGS ?= -g -Wall -ansi -pedantic -pthread
CPPFLAGS += -I$(SRCDIR)

all: tcpSocketUnit udpSocketUnit threadUnit dnsResolverUnit

$(SRCDIR)/%.o: $(SRCDIR)
	$(MAKE) -w -C $(SRCDIR) $*.o
.PHONY: $(SRCDIR)


%Unit.o: %Unit.cpp

%Unit: $(SRCDIR)/%.o %Unit.o gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@

clean:
	rm -rf *.o *.dSYM *Unit


# for emacs flymake
#
check-syntax:
	clang -Wall -Wextra -fsyntax-only -fno-show-column $(CPPFLAGS) $(CHK_SOURCES)
