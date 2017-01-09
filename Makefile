deps:
	./scripts/deps.sh build

clean-deps:
	./scripts/deps.sh clean

clean:
	rm -rf build
	rm -f *.log

base: deps
	mkdir -p build
	rm -f build/*.o build/*.a
	cd build && cmake ../

build-all: base
	cd build && make -j8

scratch: build-all
	./build/src/scratch/run_scratch

test: build-all
	./build/src/test/run_unit_tests

spinlock_bench: build-all
	./build/src/bench/spinlock_comparison

BENCHES ?= .*
mbench: build-all
	./build/src/bench/micro_benchmarks --benchmark_filter=$(BENCHES)

bench: mbench spinlock_bench

