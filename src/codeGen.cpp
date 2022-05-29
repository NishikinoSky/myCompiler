/*
 * @Author: Skyyyy
 * @Date: 2022-03-14 14:30:43
 * @Description: Heil Diana
 */
#include "codeGen.h"

codeGen* generator;

/**
 * @description: 构造函数
 * @return {*}
 */
codeGen::codeGen()
{
#if DEBUG
    std::cout << "[DEBUG] codeGen()===================" << std::endl;
#endif
    this->gModule = new llvm::Module("myCMM", theContext);
    this->printf  = this->printGen();
    // this->scanf   = this->scanGen();
}

/**
 * @description: 调用ir生成
 * @param {astNode*} root
 * @return {*}
 */
void codeGen::codeGenerator(astNode* root)
{
#if DEBUG
    std::cout << "[DEBUG] codeGenerator()===================" << std::endl;
#endif
    root->IRBuilder();
    this->gModule->print(llvm::outs(), nullptr);
}

/**
 * @description: 析构函数
 * @return {*}
 */
codeGen::~codeGen()
{
}

/**
 * @description: print函数的声明
 * @return {*}
 */
llvm::Function* codeGen::printGen()
{
    std::vector<llvm::Type*> printfArgs;
    printfArgs.push_back(theBuilder.getInt8Ty()->getPointerTo());
    llvm::ArrayRef<llvm::Type*> argsRef(printfArgs);
    llvm::FunctionType*         printfType = llvm::FunctionType::get(theBuilder.getInt32Ty(), argsRef, true);
    llvm::Function*             printfFunc = llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, llvm::Twine("printf"), gModule);
    printfFunc->setCallingConv(llvm::CallingConv::C);
    return printfFunc;
}