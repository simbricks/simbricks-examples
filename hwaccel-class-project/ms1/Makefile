include ../common/build-flags.mk

all: app/matrixmultiply app/matrixmultiply_block app/matrixmultiply_block_t

clean:
	rm -rf app/matrixmultiply app/matrixmultiply_block \
		app/matrixmultiply_block_t app/*.o out test*.out

%.out: tests/%.sim.py tests/%.check.py
	-simbricks-run --verbose --force $(SIMBRICKS_FLAGS) $<
	-python3 tests/`basename $< .sim.py`.check.py | tee $@

test0.out: app/matrixmultiply
test1.out: app/matrixmultiply
test2.out: app/matrixmultiply_block
test3.out: app/matrixmultiply_block
test4.out: app/matrixmultiply_block_t
test5.out: app/matrixmultiply_block_t

check:
	-for c in tests/*.check.py; do python3 $$c; done

test: test0.out test1.out test2.out test3.out test4.out test5.out
	cat $^

.PHONY: all clean test
