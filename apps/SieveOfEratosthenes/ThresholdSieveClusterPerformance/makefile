
CONFIG = conf.json

.SECONDEXPANSION:

all:

results: $$(all_results)

clean:
	$(RM) $(result_files) make.mk

-include make.mk

make.mk: gen_make.py $(CONFIG)
	./gen_make.py $(CONFIG) > $@


