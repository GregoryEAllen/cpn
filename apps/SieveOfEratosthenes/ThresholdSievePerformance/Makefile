
PROG = ../ThresholdSieve/ThresholdSieve
SHUFFLE = ./ShufflePerf/ShufflePerf

all: results.data.txt results.legend.txt

-include results.mk

results.txt: $(TARGETS) $(SHUFFLE)
	$(SHUFFLE) -c $(TARGETS) > $@

results.data.txt results.legend.txt: results.txt $(SHUFFLE)
	$(SHUFFLE) -s results.txt results.data.txt results.legend.txt

results.mk: test.tcl
	tclsh test.tcl

$(SHUFFLE):
	bash -c "pushd ShufflePerf; jmake; popd"

clean:
	rm -f $(TARGETS)
	rm -f results.txt results.data.txt results.legend.txt results.mk

