include Makefile.inc
export BASE = $(CURDIR)

SUBDIRS = src test doc

all: $(SUBDIRS)

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	cd $@ && $(MAKE) -w