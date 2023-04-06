DEBUG_ARGS := test.txt

all:chunk.cpp compiler.cpp debug.cpp main.cpp scanner.cpp value.cpp vm.cpp
	g++ *.cpp -o ./bin/jump -I ./include/ -g