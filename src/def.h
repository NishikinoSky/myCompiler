/*
 * @Author: Skyyyy
 * @Date: 2022-03-14 16:09:18
 * @Description: Heil Diana
 */
#pragma once
#include "llvm/ADT/SmallVector.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdarg>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <utility>
#include <vector>

// 记录了LLVM的核心数据结构，比如类型和常量表，不过不太需要关心它的内部
llvm::LLVMContext LLVMContext;
// 用于创建LLVM指令
llvm::IRBuilder<> Builder(LLVMContext);
// 用于管理函数和全局变量，类似于类c++的编译单元(单个cpp文件)
llvm::Module* gModule;
// 用于记录函数的变量参数
std::map<std::string, llvm::Value*> namedValues;

#define ERROR -1
#define VAR 0
#define INT 1
#define FLOAT 2
#define CHAR 3
#define BOOL 4
#define ARRAY 5
#define ARRAY_INT 6
#define ARRAY_FLOAT 7
#define ARRAY_CHAR 8
#define ARRAY_BOOL 9
#define VOID 10
#define FUNC 11