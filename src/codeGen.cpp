/*
 * @Author: Skyyyy
 * @Date: 2022-03-14 14:30:43
 * @Description: Heil Diana
 */
#include "codeGen.h"

/**
 * @description: print函数的声明
 * @return {*}
 */
llvm::Function* codeGen::printGen()
{
    std::vector<llvm::Type*> printfArgs;
    printfArgs.push_back(Builder.getInt8Ty()->getPointerTo());
    llvm::ArrayRef<llvm::Type*> argsRef(printfArgs);
    llvm::FunctionType*         printfType = llvm::FunctionType::get(Builder.getInt32Ty(), argsRef, true);
    llvm::Function*             printfFunc = llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, llvm::Twine("printf"), gModule);
    printfFunc->setCallingConv(llvm::CallingConv::C);
    return printfFunc;
}