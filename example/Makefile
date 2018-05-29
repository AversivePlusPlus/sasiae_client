CMAKE ?= cmake

EXE=example

all: build build/$(EXE)
	@touch build/$(EXE)

test: build build/$(EXE)
	@(echo "T 100 100" && echo "S") | ./build/$(EXE)

clean:
	@rm -rf build

mrproper: clean
	@rm -f *~
	@rm -f *.pyc

build/$(EXE): main.cpp
	@cd build && $(CMAKE) .. && make

build:
	@mkdir build
