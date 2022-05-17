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
    int                   lineIndex;   // 该节点对应的终结符/非终结符所在的行号

    astNode(char* nodeName, std::string nodeValue, int lineIndex);             // yytext存储类型为char*，用作词法分析中的leaf构建
    astNode(std::string nodeName, std::string nodeValue, int childNum, ...);   // 可变参数，用作语法分析中的其余节点构建
    ~astNode();
    // 相关值的method
    int                                               getNodeType(astNode* node);
    llvm::Type*                                       getLLVMType(int type, int size);
    std::vector<std::pair<int, std::string>>*         getVarList();
    std::vector<std::pair<llvm::Type*, std::string>>* getParamList();
    std::vector<llvm::Value*>*                        getArgList(llvm::Function* func);
    llvm::Value*                                      IRBuilder();
    llvm::Value*                                      IRBuildVar(llvm::Function* func);
    llvm::Value*                                      IRBuildFunc();
    llvm::Value*                                      IRBuildCompoundStmt(llvm::Function* func);
    llvm::Value*                                      IRBuildStmt(llvm::Function* func);
    llvm::Value*                                      IRBuildExp(llvm::Function* func);
    llvm::Value*                                      IRBuildSelec(llvm::Function* func);
    llvm::Value*                                      IRBuildIter(llvm::Function* func);
    llvm::Value*                                      IRBuildRet(llvm::Function* func);
    llvm::Value*                                      IRBuildPrint(llvm::Function* func, bool isPrintln);
    llvm::Value*                                      IRBuildScan(llvm::Function* func);
    llvm::Value*                                      IRBuildID(llvm::Function* func);
    llvm::Value*                                      typeCast(llvm::Value* elem1, llvm::Type* type2);
};
llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* func, const std::string& varName, llvm::Type* type);