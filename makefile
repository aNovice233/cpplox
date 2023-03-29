all:chunk.cpp compiler.cpp debug.cpp main.cpp scanner.cpp value.cpp vm.cpp
	g++ *.cpp -omain -I ./include/ -g