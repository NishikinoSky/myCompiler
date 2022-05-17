/*
 * @Author: Skyyyy
 * @Date: 2022-03-14 16:09:18
 * @Description: Heil Diana
 */
#pragma once
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
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

#define ERROR -1
#define VAR 0
#define VAR_INT 1
#define VAR_FLOAT 2
#define VAR_CHAR 3
#define VAR_BOOL 4
#define ARRAY 5
#define ARRAY_INT 6
#define ARRAY_FLOAT 7
#define ARRAY_CHAR 8
#define ARRAY_BOOL 9
#define VOID 10
#define FUNC 11

#define DEBUG 0