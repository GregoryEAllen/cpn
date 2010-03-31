
OS_PROG := $(shell uname -s)-$(shell uname -p)
OS := $(OS_PROG)
PROG = ../ThresholdSieve/_$(OS_PROG)/ThresholdSieve
SHUFFLE = ./ShufflePerf/_$(OS_PROG)/ShufflePerf

all: results.data.txt results.legend.txt results.pdf

-include results.mk

results.txt: $(TARGETS) $(SHUFFLE)
	$(SHUFFLE) -c $(TARGETS) > $@

results.data.txt results.legend.txt: results.txt $(SHUFFLE)
	$(SHUFFLE) -s results.txt results.data.txt results.legend.txt

results.pdf : results.data.txt results.legend.txt plotresults.py
	python plotresults.py

results.mk: test.tcl
	tclsh test.tcl

$(SHUFFLE):
	pushd ShufflePerf; jmake; popd

$(PROG):
	pushd ../ThresholdSieve; jmake; popd

clean:
	rm -f results.txt results.data.txt results.legend.txt results.mk

real-clean:
	rm -f results.txt results.data.txt results.legend.txt results.mk
	rm -f $(TARGETS)

