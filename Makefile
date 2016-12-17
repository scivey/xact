deps:
	./scripts/deps.sh build

clean-deps:
	./scripts/deps.sh clean

clean:
	rm -rf build
	rm -f *.log

base: deps
	mkdir -p build && cd build && cmake ../

clean-asm:
	rm -f build/libyup.a build/yuplib.o build/avstrlib.o build/libavstr.a

create-runner: base clean-asm
	cd build && make runner -j8

run: create-runner
	./build/runner

create-tests: base
	cd build && make unit_test_runner -j8

test: create-tests
	./build/unit_test_runner

create-bencher: clean-asm
	cd build && make bencher -j8

BENCHES ?= .*

bench: create-bencher
	./build/bencher --benchmark_filter=$(BENCHES)

.PHONY: run create-runner test create-tests bench create-bencher genasm clean-asm

genasm:
	mkdir -p tmp
	clang-3.9 -O3 -S src/nope.c -mllvm --x86-asm-syntax=intel -o tmp/nope.asm

