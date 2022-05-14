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
    std::vector<llvm::Value*>*                        getArgList();
    llvm::Value*                                      IRBuilder();
    llvm::Value*                                      IRBuildVar();
    llvm::Value*                                      IRBuildFunc();
    llvm::Value*                                      IRBuildCompoundStmt();
    llvm::Value*                                      IRBuildExp();
};

/**
 * @description: 词法分析构建leaf
 * @param {char} *nodeName 节点名称
 * @param {string} nodeValue 节点值
 * @param {int} lineIndex 节点所在行数
 * @return {*}
 */
astNode::astNode(char* nodeName, std::string nodeValue, int lineIndex)
{
    this->nodeName  = new std::string(nodeName);
    this->nodeValue = new std::string(nodeValue);
    this->lineIndex = lineIndex;
    this->childNum  = 0;
}

/**
 * @description: 语法分析构建node
 * @param {string} nodeName 节点名称，在bison中将其设为""
 * @param {string} nodeValue 节点值
 * @param {int} childNum 该节点的子节点数量
 * @return {*}
 */
astNode::astNode(std::string nodeName, std::string nodeValue, int childNum, ...)
{
    this->nodeName  = new std::string(nodeName);
    this->nodeValue = new std::string(nodeValue);
    this->childNum  = childNum;

    va_list args;
    va_start(args, childNum);
    if (childNum > 0)
    {
        for (int i = 0; i < childNum; i++)
        {
            astNode* child;
            child = va_arg(args, astNode*);
            this->childPtr.push_back(child);
        }
        this->lineIndex = this->childPtr[0]->lineIndex;
    }
    else
    {
        throw "childNum Error\n";
    }
    va_end(args);
}

/**
 * @description: 析构函数
 * @param {*}
 * @return {*}
 */
astNode::~astNode()
{
    if (this->childPtr.size() > 0)
    {
        for (int i = 0; i < this->childPtr.size(); i++)
        {
            delete this->childPtr[i];
        }
        this->childPtr.erase(this->childPtr.begin(), this->childPtr.end());
    }
    delete this;
}

/**
 * @description: 根据typeSpecifier得到对应节点类型type
 * @param {astNode} *node
 * @return nodeType
 */
int astNode::getNodeType(astNode* node)
{
    // if node->nodeValue == "typeSpecifier"
    // node->childPtr[0] int float char bool
    if (node->nodeValue->compare("typeSpecifier") == 0)
    {
        if (node->childPtr[0]->nodeValue->compare("int") == 0)
        {
            return INT;
        }
        else if (node->childPtr[0]->nodeValue->compare("float") == 0)
        {
            return FLOAT;
        }
        else if (node->childPtr[0]->nodeValue->compare("char") == 0)
        {
            return CHAR;
        }
        else if (node->childPtr[0]->nodeValue->compare("bool") == 0)
        {
            return BOOL;
        }
    }
    else
    {
        throw("getValueType Failed\n");
    }
}

/**
 * @description: 根据type得到llvmtype, 调用llvm::Type中的api
 * @param {int} type 由getValueType返回的类型
 * @param {int} size 数组大小, 非数组或指针为0, 区分ID[]与ID[int]并且作为llvm::ArrayType::get参数
 * @return llvm::Type::{*}
 */
llvm::Type* astNode::getLLVMType(int type, int size)
{
    switch (type)
    {
    case INT:
        return llvm::Type::getInt32Ty(LLVMContext);
        break;
    case FLOAT:
        return llvm::Type::getFloatTy(LLVMContext);
        break;
    case CHAR:
        return llvm::Type::getInt8Ty(LLVMContext);
        break;
    case BOOL:
        return llvm::Type::getInt1Ty(LLVMContext);
        break;
    case ARRAY_INT:
        if (size)
        {
            return llvm::ArrayType::get(llvm::Type::getInt32Ty(LLVMContext), size);
        }
        else
        {
            return llvm::Type::getInt32PtrTy(LLVMContext);
        }
        break;
    case ARRAY_FLOAT:
        if (size)
        {
            return llvm::ArrayType::get(llvm::Type::getFloatTy(LLVMContext), size);
        }
        else
        {
            return llvm::Type::getFloatPtrTy(LLVMContext);
        }
        break;
    case ARRAY_CHAR:
        if (size)
        {
            return llvm::ArrayType::get(llvm::Type::getInt8Ty(LLVMContext), size);
        }
        else
        {
            return llvm::Type::getInt8PtrTy(LLVMContext);
        }
        break;
    case ARRAY_BOOL:
        if (size)
        {
            return llvm::ArrayType::get(llvm::Type::getInt1Ty(LLVMContext), size);
        }
        else
        {
            return llvm::Type::getInt1PtrTy(LLVMContext);
        }
        break;
    default:
        // void
        return llvm::Type::getVoidTy(LLVMContext);
        break;
    }
}

/**
 * @description: 变量，得到varList并分类
 * @param {*}
 * @return std::vector<std::pair<类型, 终结符变量名ID>>* varListPtr
 * varDecList → varDef | varDef COMMA varDecList
 * varDef → ID | ID LSB INT RSB
 */
std::vector<std::pair<int, std::string>>* astNode::getVarList()
{
    astNode* node = this;
    if (node->nodeValue->compare("varDecList") == 0)
    {
        std::vector<std::pair<int, std::string>>  varList;
        std::vector<std::pair<int, std::string>>* varListPtr;
        while (true)
        {
            // ID
            if (node->childPtr[0]->childNum == 1)
            {
                varList.push_back(make_pair(VAR, *node->childPtr[0]->childPtr[0]->nodeName));
            }
            // ID LSB INT RSB
            else if (node->childPtr[0]->childNum == 4)
            {
                int arraySize = std::stoi(*node->childPtr[0]->childPtr[2]->nodeName);
                varList.push_back(make_pair(ARRAY + arraySize, *node->childPtr[0]->childPtr[0]->nodeName));
            }
            else
            {
                throw("Var Define Error\n");
            }
            // node->childPtr[2] = varDecList
            if (node->childNum == 3)
            {
                node = node->childPtr[2];
            }
            else
            {
                break;
            }
        }
        varListPtr = &varList;
        return varListPtr;
    }
    else
    {
        throw("Not varDecList\n");
    }
}



/**
 * @description: 函数形参，得到paramList
 * @param {*}
 * @return std::vector<std::pair<llvm类型, 终结符变量名ID>>* paramListPtr
 * paramList → paramDec COMMA paramList | paramDec
 * paramDec → typeSpecifier paramDef
 * paramDef → ID | ID LSB RSB
 */
std::vector<std::pair<llvm::Type*, std::string>>* astNode::getParamList()
{
    astNode* node = this;
    if (node->nodeValue->compare("paramList") == 0)
    {
        std::vector<std::pair<llvm::Type*, std::string>>  paramList;
        std::vector<std::pair<llvm::Type*, std::string>>* paramListPtr;
        while (true)
        {
            // ID
            if (node->childPtr[0]->childPtr[1]->childNum == 1)
            {
                paramList.push_back(make_pair(getLLVMType(getNodeType(node->childPtr[0]->childPtr[0]), 0), *node->childPtr[0]->childPtr[1]->childPtr[0]->nodeName));
            }
            // ID LSB RSB
            else if (node->childPtr[0]->childPtr[1]->childNum == 3)
            {
                paramList.push_back(make_pair(getLLVMType(ARRAY + getNodeType(node->childPtr[0]->childPtr[0]), 0), *node->childPtr[0]->childPtr[1]->childPtr[0]->nodeName));
            }
            else
            {
                throw("Param Error\n");
            }
            // node->childPtr[2]=paramList
            if (node->childNum == 3)
            {
                node = node->childPtr[2];
            }
            else
            {
                break;
            }
        }
        paramListPtr = &paramList;
        return paramListPtr;
    }
    else
    {
        throw("Not paramList\n");
    }
}

/**
 * @description: 得到argList
 * @param {*}
 * @return {*}
 * argList → exp COMMA argList | exp
 */
std::vector<llvm::Value*>* astNode::getArgList() {}


/**
 * @description: 自顶向下生成IR的开始，调用var与func的IR生成
 * @param {*}
 * @return {*}
 * program → decList
 * decList → dec decList | %empty
 * dec → varDeclaration | funcDeclaration
 */
llvm::Value* astNode::IRBuilder()
{
    if (this->nodeValue->compare("dec"))
    {
        if (this->childPtr[0]->nodeValue->compare("varDeclaration"))
        {
            return this->IRBuildVar();
        }
        else if (this->childPtr[0]->nodeValue->compare("funcDeclaration"))
        {
            return this->IRBuildFunc();
        }
    }
    for (int i = 0; i < this->childNum; i++)
    {
        this->childPtr[i]->IRBuilder();
    }
    return 0;
}

/**
 * @description: var相关的IR生成
 * @param {*}
 * @return {*}
 * varDeclaration → typeSpecifier varDecList SEMICOLON
 * typeSpecifier → TYPE
 * varDecList → varDef | varDef COMMA varDecList
 * varDef → ID | ID LSB INT RSB
 */
llvm::Value* astNode::IRBuildVar()
{
    std::vector<std::pair<int, std::string>>*          varList = getVarList();
    std::vector<std::pair<int, std::string>>::iterator it;
    for (it = varList->begin(); it != varList->end(); it++)
    {
        llvm::Type* llvmType;
        if (it->first == VAR)
        {
            llvmType = getLLVMType(getNodeType(this->childPtr[0]), 0);
        }
        // array
        else
        {
            llvmType = getLLVMType(getNodeType(this->childPtr[0]) + ARRAY, it->first - ARRAY);
        }

        // 全局变量
        if (1)
        {
            llvm::GlobalVariable* gVarPtr = new llvm::GlobalVariable(/*Module=*/*gModule,
                                                                     /*Type=*/llvmType,
                                                                     /*isConstant=*/false,
                                                                     /*Linkage=*/llvm::GlobalValue::CommonLinkage,
                                                                     /*Initializer=*/0,   // has initializer, specified below
                                                                     /*Name=*/it->second);
        }
    }
}

/**
 * @description: func相关的IR生成
 * @param {*}
 * @return {*}
 * funcDeclaration → typeSpecifier funcDec compoundStmt
 * funcDec → ID LPT paramList RPT | ID LPT RPT
 */
llvm::Value* astNode::IRBuildFunc()
{
    // 返回值类型
    llvm::Type* retType = getLLVMType(getNodeType(this->childPtr[0]), 0);
    // 参数类型
    std::vector<llvm::Type*>                          funcArgs;
    std::vector<std::pair<llvm::Type*, std::string>>* paramList;
    // funcDec → ID LPT paramList RPT
    if (this->childPtr[1]->childNum == 4)
    {
        paramList = this->childPtr[1]->childPtr[2]->getParamList();
        // llvm::SmallVector<llvm::Type*, static_cast<unsigned int>(paramList->size())> functionArgs;
        for (std::vector<std::pair<llvm::Type*, std::string>>::iterator it = paramList->begin(); it != paramList->end(); it++)
        {
            funcArgs.push_back(it->first);
        }
    }
    // funcDec → ID LPT RPT
    else
    {
        paramList = nullptr;
    }
    // 根据返回值类型和参数类型得到函数类型
    llvm::FunctionType* funcType = llvm::FunctionType::get(retType, funcArgs, /*isVarArg*/ false);
    // llvm::Function* func = llvm::cast<llvm::Function>(gModule->getOrInsertFunction(*this->childPtr[1]->childPtr[0]->nodeName, funcType));
    llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, *this->childPtr[1]->childPtr[0]->nodeName, gModule);
    // 存储参数（获取参数的引用）
    if (paramList != nullptr)
    {
        std::vector<std::pair<llvm::Type*, std::string>>::iterator it = paramList->begin();
        for (llvm::Function::arg_iterator argsIT = func->arg_begin(); argsIT != func->arg_end(); argsIT++)
        {
            argsIT->setName(it->second);
            it++;
        }
    }
    // 函数体
    // 创建函数的代码块
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(LLVMContext, "entry", func);
    Builder.SetInsertPoint(entry);
    this->childPtr[2]->IRBuildCompoundStmt();



    return func;
}

/**
 * @description: compoundStmt相关IR生成，函数体框架
 * @param {*}
 * @return {*}
 * compoundStmt → LCB content RCB
 * content → localDec stmtList | %empty
 * localDec → varDeclaration localDec | %empty
 * stmtList → stmt stmtList | %empty
 */
llvm::Value* astNode::IRBuildCompoundStmt()
{
    if (this->nodeValue->compare("compoundStmt") == 0)
    {
    }
    else
    {
        throw("Not compoundStmt");
    }
    return 0;
}

llvm::Value* astNode::IRBuildExp()
{
}
