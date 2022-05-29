/*
 * @Author: Skyyyy
 * @Date: 2022-03-14 13:52:46
 * @Description: Heil Diana
 */
#pragma once
#include "def.h"

class astNode
{
public:
    std::string*          nodeName;    // 存放节点的name
    std::string*          nodeValue;   // 存放节点的value
    std::vector<astNode*> childPtr;    // 存放子节点指针的vector
    int                   childNum;    // 子节点个数

    astNode(char* nodeName, std::string nodeValue);                            // yytext存储类型为char*，用作词法分析中的leaf构建
    astNode(std::string nodeName, std::string nodeValue, int childNum, ...);   // 可变参数，用作语法分析中的其余节点构建
    ~astNode();
    // 相关值的method
    int                                               getNodeType(astNode* node);
    llvm::Type*                                       getLLVMType(int type, int size, bool isArray);
    std::vector<std::pair<int, std::string>>*         getVarList();
    std::vector<std::pair<llvm::Type*, std::string>>* getParamList();
    std::vector<llvm::Value*>*                        getArgList();
    llvm::Value*                                      IRBuilder();
    llvm::Value*                                      IRBuildVar(bool isGlobal);
    llvm::Value*                                      IRBuildFunc();
    llvm::Value*                                      IRBuildCompoundStmt();
    llvm::Value*                                      IRBuildStmt();
    llvm::Value*                                      IRBuildExp();
    llvm::Value*                                      IRBuildSelec();
    llvm::Value*                                      IRBuildIter();
    llvm::Value*                                      IRBuildRet();
    llvm::Value*                                      IRBuildPrint(bool isPrintln, bool isPrintf);
    llvm::Value*                                      IRBuildScan();
    llvm::Value*                                      IRBuildID();
    llvm::Value*                                      typeCast(llvm::Value* elem1, llvm::Type* type2);
};
llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* func, const std::string& varName, llvm::Type* type);