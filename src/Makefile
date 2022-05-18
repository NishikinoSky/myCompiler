
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
	$(CC) -g $(LLVM_INCLU) $(LLVM_LIB) -lLLVM ./*.cpp -o diana

test:
	@echo "\n\n****** test1 print 1 ******"
	cat ../test/test1/test1.cmm | ./diana > ../test/test1/test1.ll
	$(LLVM_AS) ../test/test1/test1.ll -o ../test/test1/test1.bc	
	$(CC) ../test/test1/test1.bc -o ../test/test1/test1
	../test/test1/test1

clean:
	$(RM) -rf $(OBJS) gramGen.cpp gramGen.hpp diana tokenGen.cpp
	$(RM) -f ../test/*.ll ../test/*.bc ../test/test1/test1
	$(RM) -f ../test/test1/*.bc ../test/test1/*.ll

cleantest:
	$(RM) -f ../test/*.ll ../test/*.bc ../test/test1/test1
	$(RM) -f ../test/test1/*.bc ../test/test1/*.ll


