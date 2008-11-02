include Makefile.inc
export BASE = $(CURDIR)

export SRCDIR = $(BASE)/src
export BINDIR = $(BASE)/bin

SUBDIRS = src test doc

all: $(SUBDIRS)

clean:
	rm -rf $(BINDIR)/*
	-for d in $(SUBDIRS); do (cd $$d; $(MAKE) -w clean ); done

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	cd $@ && $(MAKE) -w