include Makefile.inc
export BASE = $(CURDIR)

export SRCDIR = $(BASE)/src
export BINDIR = $(BASE)/bin
export LIBDIR = $(BASE)/lib

SUBDIRS = src test doc

all: $(SUBDIRS)

clean:
	rm -rf $(BINDIR)/*
	rm -rf $(LIBDIR)/*
	-for d in $(SUBDIRS); do (cd $$d; $(MAKE) -w clean ); done

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	cd $@ && $(MAKE) -w