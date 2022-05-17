/*
 * @Author: Skyyyy
 * @Date: 2022-03-14 14:30:38
 * @Description: Heil Diana
 */
#pragma once
#include "ast.h"
#include "def.h"

// 记录了LLVM的核心数据结构，比如类型和常量表，不过不太需要关心它的内部
static llvm::LLVMContext theContext;
// 用于创建LLVM指令
static llvm::IRBuilder<> Builder(theContext);

// 用于记录函数的变量参数
// std::map<std::string, llvm::Value*> namedValues;
// 函数栈, 标记当前函数
// std::stack<llvm::Function*> funcStack;

class codeGen
{
public:
    // 用于管理函数和全局变量，类似于类c++的编译单元(单个cpp文件)
    llvm::Module*   gModule;
    llvm::Function *printf, *scanf;
    llvm::Function* printGen();
    llvm::Function* scanGen();
    void            codeGenerator(astNode* root);
    codeGen();
    ~codeGen();
};