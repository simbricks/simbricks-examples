include ../common/build-flags.mk

all: app/matmul-accel accel-sim/sim

app/matmul-accel: app/matmul-accel.o app/driver.o ../common/vfio-pci.o \
	../common/dma-alloc.o

accel-sim/sim: accel-sim/sim.o accel-sim/plumbing.o accel-sim/dma.o
accel-sim/sim: LDLIBS+=-lsimbricks

clean:
	rm -rf app/matmul-accel app/*.o accel-sim/sim accel-sim/*.o \
		out test*.out

%.out: tests/%.sim.py tests/%.check.py
	-simbricks-run --verbose --force $(SIMBRICKS_FLAGS) $<
	-python3 tests/`basename $< .sim.py`.check.py 2>&1 | tee $@

test0.out: app/matmul-accel accel-sim/sim
test1.out: app/matmul-accel accel-sim/sim

check:
	-for c in tests/*.check.py; do python3 $$c; done

test: test0.out test1.out
	cat $^

.PHONY: all clean check test
