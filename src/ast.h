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
    llvm::Value*                                      IRBuildVar(bool ifGlobalVar);
    llvm::Value*                                      IRBuildFunc();
    llvm::Value*                                      IRBuildCompoundStmt();
    llvm::Value*                                      IRBuildStmt();
    llvm::Value*                                      IRBuildExp();
    llvm::Value*                                      IRBuildSelec();
    llvm::Value*                                      IRBuildIter();
    llvm::Value*                                      IRBuildRet();
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
            return this->IRBuildVar(1);
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
llvm::Value* astNode::IRBuildVar(bool ifGlobalVar)
{
    std::vector<std::pair<int, std::string>>*          varList = getVarList();
    std::vector<std::pair<int, std::string>>::iterator it;
    for (it = varList->begin(); it != varList->end(); it++)
    {
        llvm::Type* llvmType;
        int         arraySize = 0;
        if (it->first == VAR)
        {
            llvmType = getLLVMType(getNodeType(this->childPtr[0]), 0);
        }
        // array
        else
        {
            arraySize = it->first - ARRAY;
            llvmType  = getLLVMType(getNodeType(this->childPtr[0]) + ARRAY, arraySize);
        }

        // 全局变量
        if (ifGlobalVar)
        {
            // 全局数组
            if (llvmType->isArrayTy())
            {
                llvm::GlobalVariable*        gArrayPtr = new llvm::GlobalVariable(/*Module=*/*gModule,
                                                                           /*Type=*/llvmType,
                                                                           /*isConstant=*/true,
                                                                           /*Linkage=*/llvm::GlobalValue::PrivateLinkage,
                                                                           /*Initializer=*/0,   // has initializer, specified below
                                                                           /*Name=*/it->second);
                std::vector<llvm::Constant*> constArrayElems;
                llvm::Constant*              con = llvm::ConstantInt::get(getLLVMType(getNodeType(this->childPtr[0]), 0), 0);   // 数组单个元素的类型，并且元素初始化为0
                for (int i = 0; i < arraySize; i++)
                {
                    constArrayElems.push_back(con);
                }
                llvm::ArrayType* arrayType  = llvm::ArrayType::get(getLLVMType(getNodeType(this->childPtr[0]), 0), arraySize);
                llvm::Constant*  constArray = llvm::ConstantArray::get(arrayType, constArrayElems);   //数组常量
                gArrayPtr->setInitializer(constArray);                                                //将数组常量初始化给全局常量
            }
            // 非数组
            else
            {
                llvm::GlobalVariable* gVarPtr = new llvm::GlobalVariable(/*Module=*/*gModule,
                                                                         /*Type=*/llvmType,
                                                                         /*isConstant=*/false,
                                                                         /*Linkage=*/llvm::GlobalValue::PrivateLinkage,
                                                                         /*Initializer=*/0,   // has initializer, specified below
                                                                         /*Name=*/it->second);
                llvm::Constant*       con     = llvm::ConstantInt::get(llvmType, 0);
                gVarPtr->setInitializer(con);   //对全局变量初始化，按照ir的语法是必须要初始化的
            }
        }
        // 局部变量
        else
        {
        }
    }
    return 0;
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
        // content → localDec stmtList
        if (this->childPtr[1]->childNum == 2)
        {
            // localDec → varDeclaration localDec
            astNode* localDec = this->childPtr[1]->childPtr[0];
            while (localDec->childNum == 2)
            {
                localDec->childPtr[0]->IRBuildVar(0);
                localDec = localDec->childPtr[1];
            }
            // stmtList → stmt stmtList
            astNode* stmtList = this->childPtr[1]->childPtr[1];
            while (stmtList->childNum == 2)
            {
                stmtList->childPtr[0]->IRBuildStmt();
                stmtList = stmtList->childPtr[1];
            }
        }
    }
    else
    {
        throw("Not compoundStmt");
    }
    return 0;
}

/**
 * @description: 生成stmt相关IR, 负责调用各种类型的语句的IR生成
 * @param {*}
 * @return {*}
 * stmt → expStmt | compoundStmt | selecStmt | iterStmt | retStmt
 */
llvm::Value* astNode::IRBuildStmt()
{
    if (this->childPtr[0]->nodeValue->compare("expStmt") == 0)
    {
        return this->childPtr[0]->IRBuildExp();
    }
    else if (this->childPtr[0]->nodeValue->compare("compoundStmt") == 0)
    {
        return this->childPtr[0]->IRBuildCompoundStmt();
    }
    else if (this->childPtr[0]->nodeValue->compare("selecStmt") == 0)
    {
        return this->childPtr[0]->IRBuildSelec();
    }
    else if (this->childPtr[0]->nodeValue->compare("iterStmt") == 0)
    {
        return this->childPtr[0]->IRBuildIter();
    }
    else if (this->childPtr[0]->nodeValue->compare("retStmt") == 0)
    {
        return this->childPtr[0]->IRBuildRet();
    }
    else
    {
        throw("No Fit Stmt\n");
    }
}

llvm::Value* astNode::IRBuildExp()
{
}

/**
 * @description: if相关语句IR生成
 * @param {*}
 * @return {*}
 * selecStmt → IF LPT exp RPT stmt %prec LOWER_THAN_ELSE | IF LPT exp RPT stmt ELSE stmt
 */
llvm::Value* astNode::IRBuildSelec()
{
    llvm::Value* condV            = this->childPtr[2]->IRBuildExp();
    condV                         = Builder.CreateFCmpONE(condV, llvm::ConstantFP::get(LLVMContext, llvm::APFloat(0.0)), "ifCond");
    llvm::Function*   theFunction = Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB      = llvm::BasicBlock::Create(LLVMContext, "then", theFunction);   // 自动加到函数中
    llvm::BasicBlock* elseBB      = llvm::BasicBlock::Create(LLVMContext, "else");
    llvm::BasicBlock* mergeBB     = llvm::BasicBlock::Create(LLVMContext, "ifcont");
    llvm::BranchInst* select      = Builder.CreateCondBr(condV, thenBB, elseBB);   // 插入条件分支语句的指令

    // Then语句处理
    Builder.SetInsertPoint(thenBB);
    llvm::Value* thenV = this->childPtr[4]->IRBuildStmt();
    Builder.CreateBr(mergeBB);           // 插入跳转到Merge分支的指令
    thenBB = Builder.GetInsertBlock();   // 获取Then语句的出口

    // Else语句处理
    theFunction->getBasicBlockList().push_back(elseBB);   // 添加到函数中去
    Builder.SetInsertPoint(elseBB);
    llvm::Value* elseV = nullptr;
    // selecStmt → IF LPT exp RPT stmt ELSE stmt
    if (this->childNum == 7)
    {
        elseV = this->childPtr[6]->IRBuildStmt();
    }
    Builder.CreateBr(mergeBB);           // 插入跳转到Merge分支的指令
    elseBB = Builder.GetInsertBlock();   // 获取Else语句的出口

    theFunction->getBasicBlockList().push_back(mergeBB);
    Builder.SetInsertPoint(mergeBB);

    return select;
}

/**
 * @description: while相关语句生成
 * @param {*}
 * @return {*}
 * iterStmt → WHILE LPT exp RPT stmt
 */
llvm::Value* astNode::IRBuildIter()
{
    llvm::Function*   theFunction = Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* condBB      = llvm::BasicBlock::Create(LLVMContext, "cond", theFunction);
    llvm::BasicBlock* bodyBB      = llvm::BasicBlock::Create(LLVMContext, "body", theFunction);
    llvm::BasicBlock* endBB       = llvm::BasicBlock::Create(LLVMContext, "end", theFunction);
    Builder.CreateBr(condBB);   // 跳转到条件分支

    // condBB基本块
    Builder.SetInsertPoint(condBB);
    llvm::Value* condV       = this->childPtr[2]->IRBuildExp();
    condV                    = Builder.CreateFCmpONE(condV, llvm::ConstantFP::get(LLVMContext, llvm::APFloat(0.0)), "whileCond");
    llvm::BranchInst* select = Builder.CreateCondBr(condV, bodyBB, endBB);   // 插入分支语句的指令
    condBB                   = Builder.GetInsertBlock();                     // 获取条件语句的出口

    // bodyBB基本块
    Builder.SetInsertPoint(bodyBB);
    llvm::Value* bodyV = this->childPtr[4]->IRBuildStmt();
    Builder.CreateBr(condBB);   // 跳转到condBB

    // endBB基本块
    Builder.SetInsertPoint(endBB);

    return select;
}

/**
 * @description: return & break 语句相关IR生成
 * @param {*}
 * @return {*}
 * retStmt → RETURN exp SEMICOLON | RETURN SEMICOLON | BREAK SEMICOLON
 */
llvm::Value* astNode::IRBuildRet()
{
    // retStmt → RETURN exp SEMICOLON
    if (this->childNum == 3)
    {
        llvm::Value* ret = this->childPtr[1]->IRBuildExp();
        return Builder.CreateRet(ret);
    }
    // retStmt → RETURN SEMICOLON
    else if (this->childPtr[0]->nodeValue->compare("RETURN") == 0)
    {
        return Builder.CreateRetVoid();
    }
    // retStmt → BREAK SEMICOLON
    else if (this->childPtr[0]->nodeValue->compare("BREAK") == 0)
    {
    }
    else
    {
        throw("RET ERROR\n");
    }
}