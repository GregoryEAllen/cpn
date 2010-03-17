
PROG = ../ThresholdSieve/ThresholdSieve

all: results.txt

include results.mk

results.txt: $(TARGETS)
	cat $(TARGETS) > $@


clean:
	rm -f $(TARGETS)
	rm -r results.txt

