
KERNEL:= $(shell uname -s)
CHIP:=$(shell uname -p)
OS=$(KERNEL)-$(CHIP)
OSDIR=_$(OS)

SUBDIRS:= $(shell find . -maxdepth 1 -type d -not -name '.*' -not -name '_*')
NODELISTS = $(addsuffix /$(OSDIR)/node.list,$(SUBDIRS))

.PHONY : all clean

all: node.list

clean:
	$(RM) _node.list _node.list.mk

_node.list:  $(NODELISTS)
	echo include > $@
	echo $(NODELISTS) >> $@
	echo ';' >> $@

node.list: _node.list
	ln -sf _node.list node.list

_node.list.mk:
	echo '#auto generator makefile' > $@
	for dir in $(SUBDIRS); do\
		echo -e "$$dir/$(OSDIR)/node.list: $$dir/*.cc\n\tpushd $$dir; jmake sharedlibs; popd;\n" >> $@;\
	done

-include _node.list.mk
