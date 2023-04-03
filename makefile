all:chunk.cpp compiler.cpp debug.cpp main.cpp scanner.cpp value.cpp vm.cpp
	g++ *.cpp -ocompiler -I ./include/ -g