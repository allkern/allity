SOURCES := $(wildcard src/*.c)

bin/allity main.cpp:
	mkdir -p build;
	mkdir -p bin;
	cd build; gcc -c $(addprefix ../, $(SOURCES)); cd ..;
	c++ $(wildcard *.cpp) $(wildcard build/*.o) -o bin/allity -Isrc