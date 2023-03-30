#include <stdio.h>
#include <string.h>
#include <fstream>
#include <stdlib.h>
#include "common.h" 
#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include "compiler.h"

VM vm;

static void repl(){
    char line[1024];
    for(;;){
        std::cout<<"> ";

        if(!fgets(line, sizeof(line), stdin)){
            std::cout<<std::endl;
            break;
        }
        vm.interpret(line);
        
    }
}

static std::string readFile(const std::string& path){
    std::ifstream file;
    file.open(path, std::ios::in | std::ios::binary);
    if(!file.is_open()){
        std::cerr<<"could not open file "<< path<< std::endl;
        exit(74);
    }

    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(fileSize+1, '\0');
    
    file.read(&buffer[0],fileSize);
    file.close();
    return buffer;
}

static void runFile(const std::string& path){
    std::cout<<path<<std::endl;
    std::string source = readFile(path);
    std::cout<<"source: "<<source<<std::endl;
    InterpretResult result = vm.interpret(source);

    if(result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, char* argv[]){
    if(argc == 1){
        repl();
    }else if(argc == 2){
        runFile(argv[1]);
    }else{
        fprintf(stderr, "Usage: cpplox [path]\n");
        exit(64);
    }
    return 0;
}