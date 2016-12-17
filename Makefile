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

clean-asm:
	rm -f build/*.o build/*.a

build-all: base
	cd build && make -j8

scratch: build-all
	./build/run_scratch

test: build-all
	./build/xact_test_runner

BENCHES ?= .*
bench: build-all
	./build/benchmark_runner --benchmark_filter=$(BENCHES)

