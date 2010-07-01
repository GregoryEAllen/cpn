
KERNEL:= $(shell uname -s)
CHIP:=$(shell uname -p)
OS=$(KERNEL)-$(CHIP)
OSDIR=_$(OS)

SUBDIRS:= $(shell find . -maxdepth 1 -type d -not -name '.*' -not -name '_*')
NODELISTS = $(addsuffix /$(OSDIR)/node.list,$(SUBDIRS))

.PHONY : all clean

all: node.list

clean:
	$(RM) node.list node.list.mk

real-clean: clean
	jmake clean

node.list:  $(NODELISTS) node.list.mk
	echo include > $@
	echo $(NODELISTS) >> $@
	echo ';' >> $@

node.list.mk:
	echo '#auto generator makefile' > $@
	for dir in $(SUBDIRS); do\
		echo -e "$$dir/$(OSDIR)/node.list: $$dir/*.cc\n\tpushd $$dir; jmake sharedlibs; popd;\n" >> $@;\
	done
	echo -e '\n_node.list.mk: $$(NODELISTS)\n' >> $@

-include node.list.mk
