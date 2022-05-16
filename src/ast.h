/*
 * @Author: Skyyyy
 * @Date: 2022-03-14 13:52:46
 * @Description: Heil Diana
 */
#pragma once
#include "codeGen.h"
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
    llvm::Value*                                      IRBuildOutput();
    llvm::Value*                                      IRBuildInput();
    llvm::Value*                                      IRBuildID(llvm::Function* func);
    llvm::Value*                                      typeCast(llvm::Value* elem1, llvm::Type* type2);
};
llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* func, const std::string& varName, llvm::Type* type);

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
        return llvm::Type::getInt32Ty(theContext);
        break;
    case FLOAT:
        return llvm::Type::getFloatTy(theContext);
        break;
    case CHAR:
        return llvm::Type::getInt8Ty(theContext);
        break;
    case BOOL:
        return llvm::Type::getInt1Ty(theContext);
        break;
    case ARRAY_INT:
        if (size)
        {
            return llvm::ArrayType::get(llvm::Type::getInt32Ty(theContext), size);
        }
        else
        {
            return llvm::Type::getInt32PtrTy(theContext);
        }
        break;
    case ARRAY_FLOAT:
        if (size)
        {
            return llvm::ArrayType::get(llvm::Type::getFloatTy(theContext), size);
        }
        else
        {
            return llvm::Type::getFloatPtrTy(theContext);
        }
        break;
    case ARRAY_CHAR:
        if (size)
        {
            return llvm::ArrayType::get(llvm::Type::getInt8Ty(theContext), size);
        }
        else
        {
            return llvm::Type::getInt8PtrTy(theContext);
        }
        break;
    case ARRAY_BOOL:
        if (size)
        {
            return llvm::ArrayType::get(llvm::Type::getInt1Ty(theContext), size);
        }
        else
        {
            return llvm::Type::getInt1PtrTy(theContext);
        }
        break;
    default:
        // void
        return llvm::Type::getVoidTy(theContext);
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
std::vector<llvm::Value*>* astNode::getArgList(llvm::Function* func)
{
    astNode* node = this;
    if (node->nodeValue->compare("argList") == 0)
    {
        std::vector<llvm::Value*>  argList;
        std::vector<llvm::Value*>* argListPtr;
        while (true)
        {
            if (node->childNum == 1)
            {
                argList.push_back(node->childPtr[0]->IRBuildExp(func));
                break;
            }
            else if (node->childNum == 3)
            {
                argList.push_back(node->childPtr[0]->IRBuildExp(func));
                node = node->childPtr[2];
            }
            else
            {
                throw("Arg Error\n");
            }
        }
        argListPtr = &argList;
        return argListPtr;
    }
    else
    {
        throw("Not argList\n");
    }
}


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
            return this->IRBuildVar(nullptr);
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
 * @param {llvm::Function*} func 当前所在函数
 * @return {*}
 * varDeclaration → typeSpecifier varDecList SEMICOLON
 * typeSpecifier → TYPE
 * varDecList → varDef | varDef COMMA varDecList
 * varDef → ID | ID LSB INT RSB
 */
llvm::Value* astNode::IRBuildVar(llvm::Function* func)
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
        if (func == nullptr)
        {
            // 全局变量重名
            if (gModule->getGlobalVariable(it->second, true) != nullptr)
            {
                throw("GlobalVar Name Duplicated.\n");
            }
            // 全局数组
            if (llvmType->isArrayTy())
            {
                // 全局变量插入全局变量表
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
            if (func->getValueSymbolTable()->lookup(it->second) != nullptr)
            {
                throw("LocalVar Name Duplicated.\n");
            }
            llvm::AllocaInst* varAlloca = CreateEntryBlockAlloca(func, it->second, llvmType);
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
    //  存储参数（获取参数的引用）
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
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(theContext, "entry", func);
    Builder.SetInsertPoint(entry);
    this->childPtr[2]->IRBuildCompoundStmt(func);

    return func;
}

/**
 * @description: compoundStmt相关IR生成，函数体框架
 * @param {llvm::Function*} func 当前所在函数
 * @return {*}
 * compoundStmt → LCB content RCB
 * content → localDec stmtList | %empty
 * localDec → varDeclaration localDec | %empty
 * stmtList → stmt stmtList | %empty
 */
llvm::Value* astNode::IRBuildCompoundStmt(llvm::Function* func)
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
                stmtList->childPtr[0]->IRBuildStmt(func);
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
 * @param {llvm::Function*} func 当前所在函数
 * @return {*}
 * stmt → expStmt | compoundStmt | selecStmt | iterStmt | retStmt
 */
llvm::Value* astNode::IRBuildStmt(llvm::Function* func)
{
    if (this->childPtr[0]->nodeValue->compare("expStmt") == 0)
    {
        return this->childPtr[0]->IRBuildExp(func);
    }
    else if (this->childPtr[0]->nodeValue->compare("compoundStmt") == 0)
    {
        return this->childPtr[0]->IRBuildCompoundStmt(func);
    }
    else if (this->childPtr[0]->nodeValue->compare("selecStmt") == 0)
    {
        return this->childPtr[0]->IRBuildSelec(func);
    }
    else if (this->childPtr[0]->nodeValue->compare("iterStmt") == 0)
    {
        return this->childPtr[0]->IRBuildIter(func);
    }
    else if (this->childPtr[0]->nodeValue->compare("retStmt") == 0)
    {
        return this->childPtr[0]->IRBuildRet(func);
    }
    else
    {
        throw("No Fit Stmt\n");
    }
}

/**
 * @description: 表达式相关ir生成
 * @param {llvm::Function*} func 当前所在函数
 * @return {*}
 * expStmt → exp SEMICOLON | SEMICOLON
 * exp → exp dbOper exp | sgOper exp | LPT exp RPT | ID | ID Array | ID funcCall | sgFactor
 */
llvm::Value* astNode::IRBuildExp(llvm::Function* func)
{
    // expStmt → exp SEMICOLON
    if (this->childPtr[0]->nodeValue->compare("exp") == 0)
    {
        astNode* exp = this->childPtr[0];

        // sgFactor → INT | FLOAT | CHAR | BOOL | STR
        if (exp->childPtr[0]->nodeValue->compare("sgFactor") == 0)
        {
            astNode* sgFactor = exp->childPtr[0];
            if (sgFactor->childPtr[0]->nodeValue->compare("INT") == 0)
            {
                int num = std::stoi(*sgFactor->childPtr[0]->nodeName);
                return Builder.getInt32(num);
            }
            else if (sgFactor->childPtr[0]->nodeValue->compare("FLOAT") == 0)
            {
                float num = std::stof(*sgFactor->childPtr[0]->nodeName);
                return llvm::ConstantFP::get(Builder.getFloatTy(), llvm::APFloat(num));
            }
            else if (sgFactor->childPtr[0]->nodeValue->compare("CHAR") == 0)
            {
                std::string::iterator it = sgFactor->childPtr[0]->nodeName->begin() + 1;
                // 非转义字符
                if (*it != '\\')
                {
                    return Builder.getInt8(*it);
                }
                // 转义字符
                else if (sgFactor->childPtr[0]->nodeName->compare("\a") == 0)
                {
                    return Builder.getInt8('\a');
                }
                else if (sgFactor->childPtr[0]->nodeName->compare("\b") == 0)
                {
                    return Builder.getInt8('\b');
                }
                else if (sgFactor->childPtr[0]->nodeName->compare("\f") == 0)
                {
                    return Builder.getInt8('\f');
                }
                else if (sgFactor->childPtr[0]->nodeName->compare("\n") == 0)
                {
                    return Builder.getInt8('\n');
                }
                else if (sgFactor->childPtr[0]->nodeName->compare("\r") == 0)
                {
                    return Builder.getInt8('\r');
                }
                else if (sgFactor->childPtr[0]->nodeName->compare("\t") == 0)
                {
                    return Builder.getInt8('\t');
                }
                else if (sgFactor->childPtr[0]->nodeName->compare("\v") == 0)
                {
                    return Builder.getInt8('\v');
                }
                else if (sgFactor->childPtr[0]->nodeName->compare("\\") == 0)
                {
                    return Builder.getInt8('\\');
                }
                else if (sgFactor->childPtr[0]->nodeName->compare("\?") == 0)
                {
                    return Builder.getInt8('\?');
                }
                else if (sgFactor->childPtr[0]->nodeName->compare("\'") == 0)
                {
                    return Builder.getInt8('\'');
                }
                else if (sgFactor->childPtr[0]->nodeName->compare("\"") == 0)
                {
                    return Builder.getInt8('\"');
                }
                else if (sgFactor->childPtr[0]->nodeName->compare("\0") == 0)
                {
                    return Builder.getInt8('\0');
                }
                else
                {
                    throw("exp CHAR Error\n");
                }
            }
            else if (sgFactor->childPtr[0]->nodeValue->compare("BOOL") == 0)
            {
                if (sgFactor->childPtr[0]->nodeName->compare("true") == 0)
                {
                    return Builder.getInt1(true);
                }
                else if (sgFactor->childPtr[0]->nodeName->compare("false") == 0)
                {
                    return Builder.getInt1(false);
                }
                else
                {
                    throw("exp BOOL Error\n");
                }
            }
            else if (sgFactor->childPtr[0]->nodeValue->compare("STR") == 0)
            {
                // 获取字符串, 去除\"\"
                std::string str                             = *sgFactor->childPtr[0]->nodeName;
                str                                         = str.substr(1, str.length() - 2);
                llvm::Constant*                    strConst = llvm::ConstantDataArray::getString(theContext, str, true);
                llvm::GlobalVariable*              gStrPtr  = new llvm::GlobalVariable(/*Module=*/*gModule,
                                                                         /*Type=*/strConst->getType(),
                                                                         /*isConstant=*/true,
                                                                         /*Linkage=*/llvm::GlobalValue::PrivateLinkage,
                                                                         /*Initializer=*/strConst,
                                                                         /*Name=*/"strConstTmp");
                llvm::SmallVector<llvm::Value*, 2> indexVector;
                llvm::Value*                       const0 = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(theContext), 0);
                // llvm::Value* const0 = Builder.getInt32(0);
                indexVector.push_back(const0);
                indexVector.push_back(const0);
                llvm::Value* strPtr = Builder.CreateInBoundsGEP(gStrPtr, indexVector, "arrayPtrTmp");
                return strPtr;
            }
        }
        // exp → exp dbOper exp
        // dbOper → PLUS | MINUS | MULTI | DIV | LOGIC | RELOP | ASSIGN
        else if (exp->childPtr[1]->nodeValue->compare("dbOper") == 0)
        {
            // 赋值语句
            if (exp->childPtr[1]->childPtr[0]->nodeValue->compare("ASSIGN") == 0)
            {
                llvm::Value* leftExp  = exp->childPtr[0]->IRBuildID(func);
                llvm::Value* rightExp = exp->childPtr[0]->IRBuildExp(func);
                if (rightExp->getType() != leftExp->getType()->getPointerElementType())
                {
                    rightExp = this->typeCast(rightExp, leftExp->getType()->getPointerElementType());
                }
                return Builder.CreateStore(rightExp, leftExp);
            }
            // 比较语句
            else if (exp->childPtr[1]->childPtr[0]->nodeValue->compare("RELOP") == 0)
            {
                llvm::Value* leftExp  = exp->childPtr[0]->IRBuildExp(func);
                llvm::Value* rightExp = exp->childPtr[2]->IRBuildExp(func);
                if (leftExp->getType() == llvm::Type::getInt1Ty(theContext) || rightExp->getType() == llvm::Type::getInt1Ty(theContext))
                {
                    throw("Relop oper not for bool\n");
                }
                // 类型转换 都转换成float或者int
                if (leftExp->getType() != rightExp->getType())
                {
                    if (leftExp->getType() == llvm::Type::getFloatTy(theContext))
                    {
                        rightExp = exp->typeCast(rightExp, leftExp->getType());
                    }
                    else if (rightExp->getType() == llvm::Type::getFloatTy(theContext))
                    {
                        leftExp = exp->typeCast(leftExp, rightExp->getType());
                    }
                    else if (leftExp->getType() == llvm::Type::getInt32Ty(theContext))
                    {
                        rightExp = exp->typeCast(rightExp, leftExp->getType());
                    }
                    else if (rightExp->getType() == llvm::Type::getInt32Ty(theContext))
                    {
                        leftExp = exp->typeCast(leftExp, rightExp->getType());
                    }
                }

                if (exp->childPtr[1]->childPtr[0]->nodeName->compare("<") == 0)
                {
                    if (leftExp->getType() == llvm::Type::getFloatTy(theContext))
                    {
                        return Builder.CreateFCmpOLT(leftExp, rightExp, "OLTTmp");
                    }
                    else
                    {
                        return Builder.CreateICmpSLT(leftExp, rightExp, "SLTTmp");
                    }
                }
                else if (exp->childPtr[1]->childPtr[0]->nodeName->compare("<=") == 0)
                {
                    if (leftExp->getType() == llvm::Type::getFloatTy(theContext))
                    {
                        return Builder.CreateFCmpOLE(leftExp, rightExp, "OLETmp");
                    }
                    else
                    {
                        return Builder.CreateICmpSLE(leftExp, rightExp, "SLETmp");
                    }
                }
                else if (exp->childPtr[1]->childPtr[0]->nodeName->compare(">") == 0)
                {
                    if (leftExp->getType() == llvm::Type::getFloatTy(theContext))
                    {
                        return Builder.CreateFCmpOGT(leftExp, rightExp, "OGTTmp");
                    }
                    else
                    {
                        return Builder.CreateICmpSGT(leftExp, rightExp, "SGTTmp");
                    }
                }
                else if (exp->childPtr[1]->childPtr[0]->nodeName->compare(">=") == 0)
                {
                    if (leftExp->getType() == llvm::Type::getFloatTy(theContext))
                    {
                        return Builder.CreateFCmpOGE(leftExp, rightExp, "OGETmp");
                    }
                    else
                    {
                        return Builder.CreateICmpSGE(leftExp, rightExp, "SGETmp");
                    }
                }
                else if (exp->childPtr[1]->childPtr[0]->nodeName->compare("==") == 0)
                {
                    if (leftExp->getType() == llvm::Type::getFloatTy(theContext))
                    {
                        return Builder.CreateFCmpOEQ(leftExp, rightExp, "OEQTmp");
                    }
                    else
                    {
                        return Builder.CreateICmpEQ(leftExp, rightExp, "EQTmp");
                    }
                }
                else if (exp->childPtr[1]->childPtr[0]->nodeName->compare("!=") == 0)
                {
                    if (leftExp->getType() == llvm::Type::getFloatTy(theContext))
                    {
                        return Builder.CreateFCmpONE(leftExp, rightExp, "ONETmp");
                    }
                    else
                    {
                        return Builder.CreateICmpNE(leftExp, rightExp, "NETmp");
                    }
                }
            }
            // 逻辑运算 && ||
            else if (exp->childPtr[1]->childPtr[0]->nodeValue->compare("LOGIC") == 0)
            {
                llvm::Value* leftExp  = exp->childPtr[0]->IRBuildExp(func);
                llvm::Value* rightExp = exp->childPtr[2]->IRBuildExp(func);
                if (leftExp->getType() != llvm::Type::getInt1Ty(theContext) || rightExp->getType() != llvm::Type::getInt1Ty(theContext))
                {
                    throw("Logic oper only for bool\n");
                }
                if (exp->childPtr[1]->childPtr[0]->nodeName->compare("&&") == 0)
                {
                    return Builder.CreateAnd(leftExp, rightExp, "andTmp");
                }
                else if (exp->childPtr[1]->childPtr[0]->nodeName->compare("||") == 0)
                {
                    return Builder.CreateOr(leftExp, rightExp, "orTmp");
                }
            }
            // + - * /
            else
            {
                llvm::Value* leftExp  = exp->childPtr[0]->IRBuildExp(func);
                llvm::Value* rightExp = exp->childPtr[2]->IRBuildExp(func);
                if (leftExp->getType() == llvm::Type::getInt1Ty(theContext) || rightExp->getType() == llvm::Type::getInt1Ty(theContext))
                {
                    throw("+ - * / oper not for bool\n");
                }
                // 类型转换 都转换成float或者int
                if (leftExp->getType() != rightExp->getType())
                {
                    if (leftExp->getType() == llvm::Type::getFloatTy(theContext))
                    {
                        rightExp = exp->typeCast(rightExp, leftExp->getType());
                    }
                    else if (rightExp->getType() == llvm::Type::getFloatTy(theContext))
                    {
                        leftExp = exp->typeCast(leftExp, rightExp->getType());
                    }
                    else if (leftExp->getType() == llvm::Type::getInt32Ty(theContext))
                    {
                        rightExp = exp->typeCast(rightExp, leftExp->getType());
                    }
                    else if (rightExp->getType() == llvm::Type::getInt32Ty(theContext))
                    {
                        leftExp = exp->typeCast(leftExp, rightExp->getType());
                    }
                }
                if (exp->childPtr[1]->childPtr[0]->nodeName->compare("+") == 0)
                {
                    if (leftExp->getType() == llvm::Type::getFloatTy(theContext))
                    {
                        return Builder.CreateFAdd(leftExp, rightExp, "FAddTmp");
                    }
                    else
                    {
                        return Builder.CreateAdd(leftExp, rightExp, "AddTmp");
                    }
                }
                else if (exp->childPtr[1]->childPtr[0]->nodeName->compare("-") == 0)
                {
                    if (leftExp->getType() == llvm::Type::getFloatTy(theContext))
                    {
                        return Builder.CreateFSub(leftExp, rightExp, "FSubTmp");
                    }
                    else
                    {
                        return Builder.CreateSub(leftExp, rightExp, "SubTmp");
                    }
                }
                else if (exp->childPtr[1]->childPtr[0]->nodeName->compare("*") == 0)
                {
                    if (leftExp->getType() == llvm::Type::getFloatTy(theContext))
                    {
                        return Builder.CreateFMul(leftExp, rightExp, "FMulTmp");
                    }
                    else
                    {
                        return Builder.CreateMul(leftExp, rightExp, "MulTmp");
                    }
                }
                else if (exp->childPtr[1]->childPtr[0]->nodeName->compare("/") == 0)
                {
                    if (leftExp->getType() == llvm::Type::getFloatTy(theContext))
                    {
                        return Builder.CreateFDiv(leftExp, rightExp, "FDivTmp");
                    }
                    else
                    {
                        return Builder.CreateSDiv(leftExp, rightExp, "SDivTmp");
                    }
                }
            }
        }
        // exp → sgOper exp
        // sgOper → MINUS | NOT | PLUS
        else if (exp->childPtr[0]->nodeValue->compare("sgOper") == 0)
        {
            if (exp->childPtr[0]->childPtr[0]->nodeValue->compare("MINUS") == 0)
            {
                return Builder.CreateNeg(exp->childPtr[1]->IRBuildExp(func), "negTmp");
            }
            else if (exp->childPtr[0]->childPtr[0]->nodeValue->compare("NOT") == 0)
            {
                if (exp->childPtr[1]->IRBuildExp(func)->getType() != llvm::Type::getInt1Ty(theContext))
                {
                    throw("! only for bool\n");
                }
                return Builder.CreateNot(exp->childPtr[1]->IRBuildExp(func), "notTmp");
            }
            else if (exp->childPtr[0]->childPtr[0]->nodeValue->compare("PLUS") == 0)
            {
                return exp->childPtr[1]->IRBuildExp(func);
            }
        }
        // exp → LPT exp RPT
        else if (exp->childPtr[0]->nodeValue->compare("LPT") == 0)
        {
            return exp->childPtr[1]->IRBuildExp(func);
        }
        // exp → ID | ID Array | ID funcCall
        else if (exp->childPtr[0]->nodeValue->compare("ID") == 0)
        {
            // exp → ID
            if (exp->childNum == 1)
            {
                return exp->IRBuildID(func);
            }
            // exp → ID Array
            else if (exp->childPtr[1]->nodeValue->compare("Array") == 0)
            {
                return exp->IRBuildID(func);
            }
            // exp → ID funcCall  ID()
            else if (exp->childPtr[1]->nodeValue->compare("funcCall") == 0)
            {
                // ID()
                if (exp->childPtr[1]->childNum == 2)
                {
                    // Look up the name in the global module table.
                    llvm::Function* calleeF = gModule->getFunction(*exp->childPtr[0]->nodeName);
                    if (calleeF == nullptr)
                    {
                        throw("Function Undeclared\n");
                    }
                    return Builder.CreateCall(calleeF, nullptr, "callTmp");
                }
                // ID(argList)
                else if (exp->childPtr[1]->childNum == 3)
                {
                    // Look up the name in the global module table.
                    if (exp->childPtr[0]->nodeName->compare("output"))
                    {
                        return exp->IRBuildOutput();
                    }
                    else if (exp->childPtr[0]->nodeName->compare("input"))
                    {
                        return exp->IRBuildInput();
                    }

                    llvm::Function* calleeF = gModule->getFunction(*exp->childPtr[0]->nodeName);
                    if (calleeF == nullptr)
                    {
                        throw("Function Undeclared\n");
                    }
                    std::vector<llvm::Value*>* argsV = exp->childPtr[1]->childPtr[1]->getArgList(func);
                    return Builder.CreateCall(calleeF, *argsV, "callTmp");
                }
            }
        }
    }
    // expStmt → SEMICOLON
    return NULL;
}

/**
 * @description: if相关语句IR生成
 * @param {llvm::Function*} func 当前所在函数
 * @return {*}
 * selecStmt → IF LPT exp RPT stmt %prec LOWER_THAN_ELSE | IF LPT exp RPT stmt ELSE stmt
 */
llvm::Value* astNode::IRBuildSelec(llvm::Function* func)
{
    llvm::Value* condV            = this->childPtr[2]->IRBuildExp(func);
    condV                         = Builder.CreateFCmpONE(condV, llvm::ConstantFP::get(theContext, llvm::APFloat(0.0)), "ifCond");
    llvm::Function*   theFunction = Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB      = llvm::BasicBlock::Create(theContext, "then", theFunction);   // 自动加到函数中
    llvm::BasicBlock* elseBB      = llvm::BasicBlock::Create(theContext, "else");
    llvm::BasicBlock* mergeBB     = llvm::BasicBlock::Create(theContext, "ifcond");
    llvm::BranchInst* select      = Builder.CreateCondBr(condV, thenBB, elseBB);   // 插入条件分支语句的指令

    // Then语句处理
    Builder.SetInsertPoint(thenBB);
    llvm::Value* thenV = this->childPtr[4]->IRBuildStmt(func);
    Builder.CreateBr(mergeBB);           // 插入跳转到Merge分支的指令
    thenBB = Builder.GetInsertBlock();   // 获取Then语句的出口

    // Else语句处理
    theFunction->getBasicBlockList().push_back(elseBB);   // 添加到函数中去
    Builder.SetInsertPoint(elseBB);
    llvm::Value* elseV = nullptr;
    // selecStmt → IF LPT exp RPT stmt ELSE stmt
    if (this->childNum == 7)
    {
        elseV = this->childPtr[6]->IRBuildStmt(func);
    }
    Builder.CreateBr(mergeBB);           // 插入跳转到Merge分支的指令
    elseBB = Builder.GetInsertBlock();   // 获取Else语句的出口

    theFunction->getBasicBlockList().push_back(mergeBB);
    Builder.SetInsertPoint(mergeBB);

    return select;
}

/**
 * @description: while相关语句生成
 * @param {llvm::Function*} func 当前所在函数
 * @return {*}
 * iterStmt → WHILE LPT exp RPT stmt
 */
llvm::Value* astNode::IRBuildIter(llvm::Function* func)
{
    llvm::Function*   theFunction = Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* condBB      = llvm::BasicBlock::Create(theContext, "cond", theFunction);
    llvm::BasicBlock* bodyBB      = llvm::BasicBlock::Create(theContext, "body", theFunction);
    llvm::BasicBlock* endBB       = llvm::BasicBlock::Create(theContext, "end", theFunction);
    Builder.CreateBr(condBB);   // 跳转到条件分支

    // condBB基本块
    Builder.SetInsertPoint(condBB);
    llvm::Value* condV       = this->childPtr[2]->IRBuildExp(func);
    condV                    = Builder.CreateFCmpONE(condV, llvm::ConstantFP::get(theContext, llvm::APFloat(0.0)), "whileCond");
    llvm::BranchInst* select = Builder.CreateCondBr(condV, bodyBB, endBB);   // 插入分支语句的指令
    condBB                   = Builder.GetInsertBlock();                     // 获取条件语句的出口

    // bodyBB基本块
    Builder.SetInsertPoint(bodyBB);
    llvm::Value* bodyV = this->childPtr[4]->IRBuildStmt(func);
    if (bodyV == nullptr)   // break;
    {
        Builder.CreateBr(endBB);   // 跳转到endBB
    }
    Builder.CreateBr(condBB);   // 跳转到condBB

    // endBB基本块
    Builder.SetInsertPoint(endBB);

    return select;
}

/**
 * @description: return & break 语句相关IR生成
 * @param {llvm::Function*} func 当前所在函数
 * @return {*}
 * retStmt → RETURN exp SEMICOLON | RETURN SEMICOLON | BREAK SEMICOLON
 */
llvm::Value* astNode::IRBuildRet(llvm::Function* func)
{
    // retStmt → RETURN exp SEMICOLON
    if (this->childNum == 3)
    {
        llvm::Value* ret = this->childPtr[1]->IRBuildExp(func);
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
        return nullptr;
    }
    else
    {
        throw("RET ERROR\n");
    }
}

/**
 * @description: output IR生成
 * @param {*}
 * @return {*}
 */
llvm::Value* astNode::IRBuildOutput() {}

/**
 * @description: Input IR生成
 * @param {*}
 * @return {*}
 */
llvm::Value* astNode::IRBuildInput() {}

/**
 * @description: 寻找ID的内存地址
 * @param {llvm::Function*} func
 * @return {*}
 * exp → ID | ID Array | ID funcCall
 * Array → LSB exp RSB | LSB RSB
 */
llvm::Value* astNode::IRBuildID(llvm::Function* func)
{
    //查找全局变量
    llvm::Value* var = gModule->getGlobalVariable(*this->childPtr[0]->nodeName, true);
    if (var == nullptr)
    {
        // 查找局部变量
        var = func->getValueSymbolTable()->lookup(*this->childPtr[0]->nodeName);
        if (var == nullptr)
        {
            throw("Var Undeclared\n");
        }
    }
    if (this->childNum == 2)
    {
        if (this->childPtr[1]->nodeValue->compare("Array") == 0)
        {
            // Array → LSB exp RSB
            if (this->childPtr[1]->childNum == 3)
            {
                llvm::Value*                       index  = this->childPtr[1]->childPtr[1]->IRBuildExp(func);
                llvm::Value*                       const0 = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(theContext), 0);
                llvm::SmallVector<llvm::Value*, 2> indexVector;
                indexVector.push_back(const0);
                indexVector.push_back(index);
                return Builder.CreateInBoundsGEP(var, indexVector, "arrayTmp");
            }
            else if (this->childPtr[1]->childNum == 2)
            {
                return var;
            }
        }
        else
        {
            throw("Func call cannot be left value in exp\n");
        }
    }
    // exp → ID
    else
    {
        return var;
    }
}

/**
 * @description: 二元运算隐式类型转换
 * @param {llvm::Value*} elem1 待类型转换元素
 * @param {llvm::Value*} elem2 目标类型元素
 * @return {*}
 */
llvm::Value* astNode::typeCast(llvm::Value* elem1, llvm::Type* type2)
{
    llvm::Type*                type1 = elem1->getType();
    llvm::Instruction::CastOps op;
    // int -> char
    if (type1 == llvm::Type::getInt32Ty(theContext) && type2 == llvm::Type::getInt8Ty(theContext))
    {
        op = llvm::Instruction::CastOps::Trunc;
    }
    // int -> float
    else if (type1 == llvm::Type::getInt32Ty(theContext) && type2 == llvm::Type::getFloatTy(theContext))
    {
        op = llvm::Instruction::CastOps::SIToFP;
    }
    // char -> int
    else if (type1 == llvm::Type::getInt8Ty(theContext) && type2 == llvm::Type::getInt32Ty(theContext))
    {
        op = llvm::Instruction::CastOps::ZExt;
    }
    // char -> float
    else if (type1 == llvm::Type::getInt8Ty(theContext) && type2 == llvm::Type::getFloatTy(theContext))
    {
        op = llvm::Instruction::CastOps::UIToFP;
    }
    // float -> int
    else if (type1 == llvm::Type::getFloatTy(theContext) && type2 == llvm::Type::getInt32Ty(theContext))
    {
        op = llvm::Instruction::CastOps::FPToSI;
    }
    // float -> char
    else if (type1 == llvm::Type::getFloatTy(theContext) && type2 == llvm::Type::getInt8Ty(theContext))
    {
        op = llvm::Instruction::CastOps::FPToUI;
    }
    else
    {
        throw("TypeCast Failed\n");
    }
    return Builder.CreateCast(op, elem1, type2, "typeCast");
}

/**
 * @description: 辅助函数，创建局部变量 申请内存
 * @param {llvm::Function*} func
 * @param {llvm::StringRef} varName
 * @param {llvm::Type*} type
 * @return CreateAlloca(type, 0, varName)
 */
llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* func, const std::string& varName, llvm::Type* type)
{
    llvm::IRBuilder<> tmpB(&func->getEntryBlock(),
                           func->getEntryBlock().begin());
    return tmpB.CreateAlloca(type, 0, varName);
}