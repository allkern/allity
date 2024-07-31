.PHONY: clean

SOURCES := $(wildcard src/*.c)
OS := $(wildcard build/*.o)

bin/allity main.cpp:
	mkdir -p build;
	mkdir -p bin;
	cd build; gcc -c $(addprefix ../, $(SOURCES)) -O3; cd ..;
	c++ $(wildcard *.cpp) build/*.o -o bin/allity -Isrc -O3 -g

clean:
	rm -rf build
	rm -rf bin