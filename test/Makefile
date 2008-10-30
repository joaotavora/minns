SUBDIRS = unit module

all: $(SUBDIRS)

clean:
	-for d in $(SUBDIRS); do (cd $$d; $(MAKE) -w clean ); done

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	cd $@ && $(MAKE) -w


