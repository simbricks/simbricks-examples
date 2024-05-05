include ../common/build-flags.mk

SIM_OBJS := accel-sim/plumbing.o accel-sim/dma.o

all: app/matmul-accel hw_comb/sim hw_vec/sim

app/matmul-accel: app/matmul-accel.o app/driver.o ../common/vfio-pci.o \
	../common/dma-alloc.o

%/obj_dir/Vtop.cpp: %/top.v accel-sim/sim.cpp $(SIM_OBJS)
	LANGUAGE=C LC_ALL=C LANG=C verilator --cc -O3 -Wall --trace \
		--Mdir $(dir $<)/obj_dir -I$(dir $<) \
		-CFLAGS "-I/simbricks/lib -I$(abspath common)"\
		-CFLAGS "-I$(abspath $(dir $<)/obj_dir)" \
		-LDFLAGS "-L/simbricks/lib" \
		-LDFLAGS "$(abspath accel-sim/plumbing.o accel-sim/dma.o) -lsimbricks" \
		$(dir $<)/top.v \
		--exe $(abspath accel-sim/sim.cpp)

# manually add additional modules as dependencies
hw_comb/obj_dir/Vtop.cpp: hw_comb/matmul_comb.v
hw_vec/obj_dir/Vtop.cpp: hw_vec/matmul_vec.v hw_vec/matmul_fetch.v \
	hw_vec/matmul_calc.v

%/sim: %/obj_dir/Vtop.cpp accel-sim/sim.cpp $(SIM_OBJS) common/reg_defs.h
	LANGUAGE=C LC_ALL=C LANG=C $(MAKE) -C $(dir $@)/obj_dir -f Vtop.mk
	cp $(dir $@)/obj_dir/Vtop $@

clean:
	rm -rf app/matmul-accel app/*.o accel-sim/sim accel-sim/*.o \
		out test*.out hw_*/obj_dir hw_*/sim

%.out: tests/%.sim.py tests/%.check.py
	-simbricks-run --verbose --force $(SIMBRICKS_FLAGS) $<
	-python3 tests/`basename $< .sim.py`.check.py 2>&1 | tee $@

test0.out: app/matmul-accel hw_comb/sim
test1.out: app/matmul-accel hw_vec/sim
test2.out: app/matmul-accel hw_comb/sim hw_vec/sim

check:
	-for c in tests/*.check.py; do python3 $$c; done

test: test0.out test1.out test2.out
	cat $^

.PHONY: all clean check test
