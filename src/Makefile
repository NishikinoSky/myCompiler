
NAME 		= cmm
CC			= clang++ -std=c++17
OBJS 		= gramGen.o tokenGen.o main.o codeGen.o ast.o
LLVM_DIR 	= /opt/homebrew/opt/llvm
LLVM_CONFIG = ${LLVM_DIR}/bin/llvm-config
LLVM_OPT    = ${LLVM_DIR}/bin/opt
LLVM_DIS    = ${LLVM_DIR}/bin/llvm-dis
LLVM_AS 	= ${LLVM_DIR}/bin/llvm-as
LLVM_LLI    = ${LLVM_DIR}/bin/lli
LLVM_LLC    = ${LLVM_DIR}/bin/llc
LLVM_CLANG  = ${LLVM_DIR}/bin/clang
LLVM_FMT    = ${LLVM_DIR}/bin/clang-format
LLVM_LIB 	= -L ${LLVM_DIR}/lib
LLVM_INCLU	= -I ${LLVM_DIR}/include

.PHONY: clean

build:
	flex -o tokenGen.cpp tokenGen.l
	bison -d -o gramGen.cpp gramGen.y
	$(CC) $(LLVM_INCLU) $(LLVM_LIB) -lLLVM ./*.cpp -o diana

test:
	cat ../test/test.cmm | ./diana > ../test/test.ll
	$(LLVM_AS) ../test/test.ll -o ../test/test.bc	
	$(CC) ../test/test.bc -o ../test/test
	../test/test
	../test/test > ../test/result.txt
clean:
	$(RM) -f $(OBJS) gramGen.cpp gramGen.hpp diana tokenGen.cpp
	$(RM) -f ../test/*.ll ../test/*.bc ../test/test
	$(RM) -f astVis/treeData.json

cleantest:
	$(RM) -f ../test/*.ll ../test/*.bc ../test/test


