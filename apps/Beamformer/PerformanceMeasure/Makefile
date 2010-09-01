
DIR=..
REPS=100
VBF=$(DIR)/fvBeamformer -v $(DIR)/coeffs.vbf.data -i $(DIR)/input.vbftest.data -n -a 5 -r $(REPS)
HBF=$(DIR)/hBeamformer -h $(DIR)/coeffs.hbf.data -i $(DIR)/input.hbftest.data -n -r $(REPS)
OMPBF=$(DIR)/ompbf -v $(DIR)/coeffs.vbf.data -h $(DIR)/coeffs.hbf.data -i $(DIR)/input.vbftest.data -n -a 5 -r $(REPS)
CPNBF=$(DIR)/cpnbf -v $(DIR)/coeffs.vbf.data -h $(DIR)/coeffs.hbf.data -i $(DIR)/input.vbftest.data -n -a 4 -r $(REPS)

define make_data
	OMP_WAIT_POLICY=$(2) /usr/bin/time -v $($(1)) -p $(3) 2>&1 | tee $(4)
endef

define make_data_spin
	GOMP_SPINCOUNT=$(2) /usr/bin/time -v $($(1)) -p $(3) 2>&1 | tee $(4)
endef

compute_format = awk -F '[[:space:].:]*' '{ printf "%02d %s.%s\n", $$2, $$5, $$6 }' | sort
define compute_avg
	egrep 'Avg:' $(1) | $(compute_format) > $(2)
endef
define compute_min
	egrep 'Min:' $(1) | $(compute_format) > $(2)
endef
define compute_max
	egrep 'Max:' $(1) | $(compute_format) > $(2)
endef


TARGETS += result.full.avg.pdf result.half.avg.pdf
TARGETS += result.full.min.pdf result.half.min.pdf
TARGETS += result.full.max.pdf result.half.max.pdf

all: $(TARGETS)

clean:
	$(RM) $(TARGETS) $(intermediate_files)

real-clean: clean
	$(RM) $(data_files) data.mk

data.mk: gen_data_mk.py
	python gen_data_mk.py > $@

include data.mk

data: $(data_files)

intermediate: $(intermediate_files)

