/*
 * @Author: Skyyyy
 * @Date: 2022-03-14 13:52:59
 * @Description: Heil Diana
 */
#include "ast.h"
#include "codeGen.h"

astNode*        astRoot;
extern codeGen* generator;

/**
 * @description: 词法分析构建leaf
 * @param {char} *nodeName 节点名称
 * @param {string} nodeValue 节点值
 * @param {int} lineIndex 节点所在行数
 * @return {*}
 */
astNode::astNode(char* nodeName, std::string nodeValue)
{
    this->nodeName  = new std::string(nodeName);
    this->nodeValue = new std::string(nodeValue);
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
            this->childPtr.push_back(va_arg(args, astNode*));
        }
    }
    else
    {
        throw("childNum Error\n");
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
 * @description: 根据typeSpecifier得到对应节点类型TYPE
 * @param {astNode} *node
 * @return nodeType
 * typeSpecifier → TYPE
 */
int astNode::getNodeType(astNode* node)
{
    // if node->nodeValue == "typeSpecifier"
    // node->childPtr[0] int float char bool
    if (node->nodeValue->compare("typeSpecifier") == 0)
    {
        if (node->childPtr[0]->nodeName->compare("int") == 0)
        {
            return VAR_INT;
        }
        else if (node->childPtr[0]->nodeName->compare("float") == 0)
        {
            return VAR_FLOAT;
        }
        else if (node->childPtr[0]->nodeName->compare("char") == 0)
        {
            return VAR_CHAR;
        }
        else if (node->childPtr[0]->nodeName->compare("bool") == 0)
        {
            return VAR_BOOL;
        }
        else if (node->childPtr[0]->nodeName->compare("void") == 0)
        {
            return VAR_VOID;
        }
    }
    else
    {
        throw("getValueType Failed\n");
    }
    return -1;
}

/**
 * @description: 根据type得到llvmtype, 调用llvm::Type中的api
 * @param {int} type 由getValueType返回的类型
 * @param {int} size 数组大小, 非数组或指针为0, 区分ID[](ptr)与ID[int](array)并且作为llvm::ArrayType::get参数
 * @param {bool} isArray 判断是否为数组
 * @return llvm::Type::{*}
 */
llvm::Type* astNode::getLLVMType(int type, int size, bool isArray)
{

    switch (type)
    {
    case VAR_INT:
        if (!isArray)
        {
            return theBuilder.getInt32Ty();
        }
        else
        {
            if (size)
            {
                return llvm::ArrayType::get(theBuilder.getInt32Ty(), size);
            }
            else
            {
                return llvm::Type::getInt32PtrTy(theContext);
            }
        }
        break;
    case VAR_FLOAT:
        if (!isArray)
        {
            return theBuilder.getFloatTy();
        }
        else
        {
            if (size)
            {
                return llvm::ArrayType::get(theBuilder.getFloatTy(), size);
            }
            else
            {
                return llvm::Type::getFloatPtrTy(theContext);
            }
        }
        break;
    case VAR_CHAR:
        if (!isArray)
        {
            return theBuilder.getInt8Ty();
        }
        else
        {
            if (size)
            {
                return llvm::ArrayType::get(theBuilder.getInt8Ty(), size);
            }
            else
            {
                return llvm::Type::getInt8PtrTy(theContext);
            }
        }
        break;
    case VAR_BOOL:
        if (!isArray)
        {
            return theBuilder.getInt1Ty();
        }
        else
        {
            if (size)
            {
                return llvm::ArrayType::get(theBuilder.getInt1Ty(), size);
            }
            else
            {
                return llvm::Type::getInt1PtrTy(theContext);
            }
        }
        break;
    default:
        // void
        return theBuilder.getVoidTy();
        break;
    }
}

/**
 * @description: 变量，得到varList并分类
 * @param {*}
 * @return std::vector<std::pair<类型, 终结符变量名ID>>* varListPtr
 * varDecList → varDef | varDef COMMA varDecList
 * varDef → ID | ID LSB INT RSB | ID LSB RSB
 */
std::vector<std::pair<int, std::string>>* astNode::getVarList()
{
    astNode* node = this;
    if (node->nodeValue->compare("varDecList") == 0)
    {
        // std::vector<std::pair<int, std::string>>  varList;
        std::vector<std::pair<int, std::string>>* varListPtr = new std::vector<std::pair<int, std::string>>;
        while (true)
        {
            // ID
            if (node->childPtr[0]->childNum == 1)
            {
                varListPtr->push_back(make_pair(VAR, *node->childPtr[0]->childPtr[0]->nodeName));
            }
            // ID LSB RSB
            else if (node->childPtr[0]->childNum == 3)
            {
                varListPtr->push_back(make_pair(ARRAY, *node->childPtr[0]->childPtr[0]->nodeName));
            }
            // ID LSB INT RSB
            else if (node->childPtr[0]->childNum == 4)
            {
                int arraySize = std::stoi(*node->childPtr[0]->childPtr[2]->nodeName);
                varListPtr->push_back(make_pair(ARRAY + arraySize, *node->childPtr[0]->childPtr[0]->nodeName));
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
        // varListPtr = &varList;
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
 * paramDec → typeSpecifier varDef
 */
std::vector<std::pair<llvm::Type*, std::string>>* astNode::getParamList()
{
    astNode* node = this;
    if (node->nodeValue->compare("paramList") == 0)
    {
        // std::vector<std::pair<llvm::Type*, std::string>>  paramList;
        std::vector<std::pair<llvm::Type*, std::string>>* paramListPtr = new std::vector<std::pair<llvm::Type*, std::string>>;
        while (true)
        {
            // ID
            if (node->childPtr[0]->childPtr[1]->childNum == 1)
            {
                paramListPtr->push_back(make_pair(getLLVMType(getNodeType(node->childPtr[0]->childPtr[0]), 0, false), *node->childPtr[0]->childPtr[1]->childPtr[0]->nodeName));
            }
            // ID LSB RSB
            else if (node->childPtr[0]->childPtr[1]->childNum == 3)
            {
                paramListPtr->push_back(make_pair(getLLVMType(getNodeType(node->childPtr[0]->childPtr[0]), 0, true), *node->childPtr[0]->childPtr[1]->childPtr[0]->nodeName));
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
        // paramListPtr = &paramList;
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
std::vector<llvm::Value*>* astNode::getArgList()
{
    astNode* node = this;
    if (node->nodeValue->compare("argList") == 0)
    {
        // std::vector<llvm::Value*>  argList;
        std::vector<llvm::Value*>* argListPtr = new std::vector<llvm::Value*>;
        while (true)
        {
            argListPtr->push_back(node->childPtr[0]->IRBuildExp());
            if (node->childNum == 3)
            {
                // argList → exp COMMA argList
                node = node->childPtr[2];
            }
            else
            {
                break;
            }
        }
        // argListPtr = &argList;
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
#if DEBUG
    std::cout << "[DEBUG] IRBuilder() start ===================================================" << std::endl;
#endif
    if (this->nodeValue->compare("program") == 0)
    {
        this->childPtr[0]->IRBuilder();
    }
    else if (this->nodeValue->compare("decList") == 0)
    {
        if (this->childNum == 2)
        {
            if (this->childPtr[0] != nullptr)
            {
                this->childPtr[0]->IRBuilder();
            }

            if (this->childPtr[1] != nullptr)
            {
                this->childPtr[1]->IRBuilder();
            }
        }
    }
    else if (this->nodeValue->compare("dec") == 0)
    {
        if (this->childPtr[0]->nodeValue->compare("varDeclaration") == 0)
        {
            return this->childPtr[0]->IRBuildVar(true);
        }
        else if (this->childPtr[0]->nodeValue->compare("funcDeclaration") == 0)
        {
            return this->childPtr[0]->IRBuildFunc();
        }
    }
#if DEBUG
    std::cout << "[DEBUG] IRBuilder() finish ==================================================" << std::endl;
#endif
    return NULL;
}

/**
 * @description: var相关的IR生成
 * @param {llvm::Function*} func 当前所在函数
 * @return {*}
 * varDeclaration → typeSpecifier varDecList SEMICOLON
 * typeSpecifier → TYPE
 * varDecList → varDef | varDef COMMA varDecList
 * varDef → ID | ID LSB INT RSB | ID LSB RSB
 */
llvm::Value* astNode::IRBuildVar(bool isGlobal)
{

    std::vector<std::pair<int, std::string>>* varList = this->childPtr[1]->getVarList();
    // std::vector<std::pair<int, std::string>>::iterator it;
    // for (it = varList->begin(); it != varList->end(); it++)
    for (auto it : *varList)
    {
        llvm::Type* llvmType;
        int         arraySize = 0;
        if (it.first == VAR)
        {
            llvmType = getLLVMType(getNodeType(this->childPtr[0]), 0, false);
        }
        // array
        else
        {
            arraySize = it.first - ARRAY;
            llvmType  = getLLVMType(getNodeType(this->childPtr[0]), arraySize, true);
        }
        // 全局变量
        if (isGlobal)
        {
            // 全局变量重名
            if (generator->gModule->getGlobalVariable(it.second, true) != nullptr)
            {
                throw("GlobalVar Name Duplicated.\n");
            }
            // 全局数组
            if (llvmType->isArrayTy())
            {
                // 全局变量插入全局变量表
                llvm::GlobalVariable*        gArrayPtr = new llvm::GlobalVariable(/*Module=*/*generator->gModule,
                                                                           /*Type=*/llvmType,
                                                                           /*isConstant=*/false,
                                                                           /*Linkage=*/llvm::GlobalValue::PrivateLinkage,
                                                                           /*Initializer=*/0,   // has initializer, specified below
                                                                           /*Name=*/it.second);
                std::vector<llvm::Constant*> constArrayElems;
                llvm::Constant*              con = llvm::ConstantInt::get(llvmType->getArrayElementType(), 0);   // 数组单个元素的类型，并且元素初始化为0
                for (int i = 0; i < arraySize; i++)
                {
                    constArrayElems.push_back(con);
                }
                llvm::ArrayType* arrayType  = llvm::ArrayType::get(llvmType->getArrayElementType(), arraySize);
                llvm::Constant*  constArray = llvm::ConstantArray::get(arrayType, constArrayElems);   //数组常量
                gArrayPtr->setInitializer(constArray);                                                //将数组常量初始化给全局常量
            }
            // 非数组
            else
            {
                llvm::GlobalVariable* gVarPtr = new llvm::GlobalVariable(/*Module=*/*generator->gModule,
                                                                         /*Type=*/llvmType,
                                                                         /*isConstant=*/false,
                                                                         /*Linkage=*/llvm::GlobalValue::PrivateLinkage,
                                                                         /*Initializer=*/0,   // has initializer, specified below
                                                                         /*Name=*/it.second);
                llvm::Constant*       con     = llvm::ConstantInt::get(llvmType, 0);
                gVarPtr->setInitializer(con);   //对全局变量初始化，按照ir的语法是必须要初始化的
            }
        }
        // 局部变量
        else
        {
#if DEBUG
            std::cout << "start Build Local Var...." << std::endl;
#endif
            if (theBuilder.GetInsertBlock()->getParent()->getValueSymbolTable()->lookup(it.second) != nullptr)
            {
                throw("LocalVar Name Duplicated.\n");
            }
            llvm::AllocaInst* varAlloca = CreateEntryBlockAlloca(theBuilder.GetInsertBlock()->getParent(), it.second, llvmType);
        }
    }
    return NULL;
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
    llvm::Type* retType = getLLVMType(getNodeType(this->childPtr[0]), 0, false);
    // 参数类型
    std::vector<llvm::Type*>                          funcArgType;
    std::vector<std::pair<llvm::Type*, std::string>>* paramList = new std::vector<std::pair<llvm::Type*, std::string>>;
    // funcDec → ID LPT paramList RPT
    if (this->childPtr[1]->childNum == 4)
    {
        paramList = this->childPtr[1]->childPtr[2]->getParamList();
        // llvm::SmallVector<llvm::Type*, static_cast<unsigned int>(paramList->size())> functionArgs;
        // for (std::vector<std::pair<llvm::Type*, std::string>>::iterator it = paramList->begin(); it != paramList->end(); it++)
        for (auto it : *paramList)
        {
            funcArgType.push_back(it.first);
        }
    }
    else
    {
        // funcDec → ID LPT RPT
        paramList = nullptr;
    }

    // 根据返回值类型和参数类型得到函数类型
    llvm::FunctionType* funcType = llvm::FunctionType::get(retType, funcArgType, /*isVarArg*/ false);
    // llvm::Function*     func     = cast<llvm::Function>(generator->gModule->getOrInsertFunction(*this->childPtr[1]->childPtr[0]->nodeName, funcType));
    llvm::Function* func = llvm::Function::Create(funcType, llvm::GlobalValue::ExternalLinkage, *this->childPtr[1]->childPtr[0]->nodeName, generator->gModule);

    //   存储参数（获取参数的引用）
    if (paramList != nullptr)
    {
        std::vector<std::pair<llvm::Type*, std::string>>::iterator it = paramList->begin();
        for (llvm::Function::arg_iterator argsIT = func->arg_begin(); argsIT != func->arg_end(); argsIT++)
        {
            llvm::Value* arg = argsIT;
            arg->setName(it->second);
            it++;
        }
    }
    // 函数体
    // 创建函数的代码块
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(theContext, "entry", func);
    theBuilder.SetInsertPoint(entry);
    this->childPtr[2]->IRBuildCompoundStmt();
    return func;
}

/**
 * @description: compoundStmt相关IR生成，函数体框架
 * @param {llvm::Function*} func 当前所在函数
 * @return {*}
 * compoundStmt → LCB localDec stmtList RCB
 * localDec → varDeclaration localDec | %empty
 * stmtList → stmt stmtList | %empty
 */
llvm::Value* astNode::IRBuildCompoundStmt()
{
#if DEBUG
    std::cout << "===============compoundStmt" << std::endl;
#endif
    if (this->nodeValue->compare("compoundStmt") == 0)
    {

        // localDec → varDeclaration localDec
        astNode* localDec = this->childPtr[1];
        while (localDec != nullptr && localDec->childNum == 2)
        {
            localDec->childPtr[0]->IRBuildVar(false);
            localDec = localDec->childPtr[1];
        }

        // stmtList → stmt stmtList
        astNode* stmtList = this->childPtr[2];
        while (stmtList != nullptr && stmtList->childNum == 2)
        {
            stmtList->childPtr[0]->IRBuildStmt();
            stmtList = stmtList->childPtr[1];
        }
    }
    else
    {
        throw("Not compoundStmt");
    }
    return NULL;
}

/**
 * @description: 生成stmt相关IR, 负责调用各种类型的语句的IR生成
 * @param {llvm::Function*} func 当前所在函数
 * @return {*}
 * stmt → expStmt | compoundStmt | selecStmt | iterStmt | retStmt
 */
llvm::Value* astNode::IRBuildStmt()
{
    if (this->childPtr[0]->nodeValue->compare("expStmt") == 0)
    {
        // expStmt → exp SEMICOLON | SEMICOLON
        if (this->childPtr[0]->childNum == 2)
        {
            return this->childPtr[0]->childPtr[0]->IRBuildExp();
        }
        else
        {
            // ;
            return NULL;
        }
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

/**
 * @description: 表达式相关ir生成
 * @param {llvm::Function*} func 当前所在函数
 * @return {*}
 * exp → exp dbOper exp | sgOper exp | LPT exp RPT | ID | ID Array | ID funcCall | sgFactor
 */
llvm::Value* astNode::IRBuildExp()
{
    // expStmt → exp SEMICOLON

    // exp → ID
    if (this->childPtr[0]->nodeValue->compare("ID") == 0 && this->childNum == 1)
    {
        //查找局部变量
        llvm::Value* var = this->IRBuildID();
        if (var->getType()->isPointerTy())
        {
            return theBuilder.CreateLoad(var->getType()->getPointerElementType(), var, "tmpVar");
        }
        else
        {
            return var;
        }
    }
    // sgFactor → INT | FLOAT | CHAR | BOOL | STR
    if (this->childPtr[0]->nodeValue->compare("sgFactor") == 0)
    {
        astNode* sgFactor = this->childPtr[0];
        if (sgFactor->childPtr[0]->nodeValue->compare("INT") == 0)
        {
            int num = std::stoi(*sgFactor->childPtr[0]->nodeName);
            return llvm::ConstantInt::get(theBuilder.getInt32Ty(), num);
        }
        else if (sgFactor->childPtr[0]->nodeValue->compare("FLOAT") == 0)
        {
            float num = std::stof(*sgFactor->childPtr[0]->nodeName);
            return llvm::ConstantFP::get(theBuilder.getFloatTy(), llvm::APFloat(num));
        }
        else if (sgFactor->childPtr[0]->nodeValue->compare("CHAR") == 0)
        {
            std::string::iterator it = sgFactor->childPtr[0]->nodeName->begin() + 1;
            //  非转义字符
            if (*it != '\\')
            {
                return theBuilder.getInt8(*it);
            }
            // 转义字符
            else if (sgFactor->childPtr[0]->nodeName->compare("'\\a'") == 0)
            {
                return theBuilder.getInt8('\a');
            }
            else if (sgFactor->childPtr[0]->nodeName->compare("'\\b'") == 0)
            {
                return theBuilder.getInt8('\b');
            }
            else if (sgFactor->childPtr[0]->nodeName->compare("'\\f'") == 0)
            {
                return theBuilder.getInt8('\f');
            }
            else if (sgFactor->childPtr[0]->nodeName->compare("'\\n'") == 0)
            {
                return theBuilder.getInt8('\n');
            }
            else if (sgFactor->childPtr[0]->nodeName->compare("'\\r'") == 0)
            {
                return theBuilder.getInt8('\r');
            }
            else if (sgFactor->childPtr[0]->nodeName->compare("'\\t'") == 0)
            {
                return theBuilder.getInt8('\t');
            }
            else if (sgFactor->childPtr[0]->nodeName->compare("'\\v'") == 0)
            {
                return theBuilder.getInt8('\v');
            }
            else if (sgFactor->childPtr[0]->nodeName->compare("'\\\\'") == 0)
            {
                return theBuilder.getInt8('\\');
            }
            else if (sgFactor->childPtr[0]->nodeName->compare("'\\?'") == 0)
            {
                return theBuilder.getInt8('\?');
            }
            else if (sgFactor->childPtr[0]->nodeName->compare("'\\''") == 0)
            {
                return theBuilder.getInt8('\'');
            }
            else if (sgFactor->childPtr[0]->nodeName->compare("'\\\"'") == 0)
            {
                return theBuilder.getInt8('\"');
            }
            else if (sgFactor->childPtr[0]->nodeName->compare("'\\0'") == 0)
            {
                return theBuilder.getInt8('\0');
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
                return theBuilder.getInt1(true);
            }
            else if (sgFactor->childPtr[0]->nodeName->compare("false") == 0)
            {
                return theBuilder.getInt1(false);
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
            auto                               gStrPtr  = new llvm::GlobalVariable(/*Module=*/*generator->gModule,
                                                    /*Type=*/strConst->getType(),
                                                    /*isConstant=*/true,
                                                    /*Linkage=*/llvm::GlobalValue::PrivateLinkage,
                                                    /*Initializer=*/strConst,
                                                    /*Name=*/"strConstTmp");
            llvm::SmallVector<llvm::Value*, 2> indexVector;
            llvm::Value*                       const0 = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(theContext), 0);
            indexVector.push_back(const0);
            indexVector.push_back(const0);
            llvm::ArrayRef<llvm::Value*> indexList(indexVector);
            llvm::Value*                 strPtr = theBuilder.CreateGEP(gStrPtr, indexList, "arrayPtrTmp");
            return strPtr;
        }
    }
    // exp → exp dbOper exp
    // dbOper → PLUS | MINUS | MULTI | DIV | LOGIC | RELOP | ASSIGN
    if (this->childPtr[1]->nodeValue->compare("dbOper") == 0)
    {
        // 赋值语句
        if (this->childPtr[1]->childPtr[0]->nodeValue->compare("ASSIGN") == 0)
        {
            llvm::Value* leftExp  = this->childPtr[0]->IRBuildID();
            llvm::Value* rightExp = this->childPtr[2]->IRBuildExp();
            if (rightExp->getType() != leftExp->getType()->getPointerElementType())
            {
                rightExp = this->typeCast(rightExp, leftExp->getType()->getPointerElementType());
            }
            return theBuilder.CreateStore(rightExp, leftExp);
        }
        // 比较语句
        else if (this->childPtr[1]->childPtr[0]->nodeValue->compare("RELOP") == 0)
        {
            llvm::Value* leftExp  = this->childPtr[0]->IRBuildExp();
            llvm::Value* rightExp = this->childPtr[2]->IRBuildExp();
            if (leftExp->getType() == theBuilder.getInt1Ty() || rightExp->getType() == theBuilder.getInt1Ty())
            {
                throw("Relop oper not for bool\n");
            }
            // 类型转换 都转换成float或者int
            if (leftExp->getType() != rightExp->getType())
            {
                if (leftExp->getType() == theBuilder.getFloatTy())
                {
                    rightExp = this->typeCast(rightExp, leftExp->getType());
                }
                else if (rightExp->getType() == theBuilder.getFloatTy())
                {
                    leftExp = this->typeCast(leftExp, rightExp->getType());
                }
                else if (leftExp->getType() == theBuilder.getInt32Ty())
                {
                    rightExp = this->typeCast(rightExp, leftExp->getType());
                }
                else if (rightExp->getType() == theBuilder.getInt32Ty())
                {
                    leftExp = this->typeCast(leftExp, rightExp->getType());
                }
            }

            if (this->childPtr[1]->childPtr[0]->nodeName->compare("<") == 0)
            {
                if (leftExp->getType() == theBuilder.getFloatTy())
                {
                    return theBuilder.CreateFCmpOLT(leftExp, rightExp, "OLTTmp");
                }
                else
                {
                    return theBuilder.CreateICmpSLT(leftExp, rightExp, "SLTTmp");
                }
            }
            else if (this->childPtr[1]->childPtr[0]->nodeName->compare("<=") == 0)
            {
                if (leftExp->getType() == theBuilder.getFloatTy())
                {
                    return theBuilder.CreateFCmpOLE(leftExp, rightExp, "OLETmp");
                }
                else
                {
                    return theBuilder.CreateICmpSLE(leftExp, rightExp, "SLETmp");
                }
            }
            else if (this->childPtr[1]->childPtr[0]->nodeName->compare(">") == 0)
            {
                if (leftExp->getType() == theBuilder.getFloatTy())
                {
                    return theBuilder.CreateFCmpOGT(leftExp, rightExp, "OGTTmp");
                }
                else
                {
                    return theBuilder.CreateICmpSGT(leftExp, rightExp, "SGTTmp");
                }
            }
            else if (this->childPtr[1]->childPtr[0]->nodeName->compare(">=") == 0)
            {
                if (leftExp->getType() == theBuilder.getFloatTy())
                {
                    return theBuilder.CreateFCmpOGE(leftExp, rightExp, "OGETmp");
                }
                else
                {
                    return theBuilder.CreateICmpSGE(leftExp, rightExp, "SGETmp");
                }
            }
            else if (this->childPtr[1]->childPtr[0]->nodeName->compare("==") == 0)
            {
                if (leftExp->getType() == theBuilder.getFloatTy())
                {
                    return theBuilder.CreateFCmpOEQ(leftExp, rightExp, "OEQTmp");
                }
                else
                {
                    return theBuilder.CreateICmpEQ(leftExp, rightExp, "EQTmp");
                }
            }
            else if (this->childPtr[1]->childPtr[0]->nodeName->compare("!=") == 0)
            {
                if (leftExp->getType() == theBuilder.getFloatTy())
                {
                    return theBuilder.CreateFCmpONE(leftExp, rightExp, "ONETmp");
                }
                else
                {
                    return theBuilder.CreateICmpNE(leftExp, rightExp, "NETmp");
                }
            }
        }
        // 逻辑运算 && ||
        else if (this->childPtr[1]->childPtr[0]->nodeValue->compare("LOGIC") == 0)
        {
            llvm::Value* leftExp  = this->childPtr[0]->IRBuildExp();
            llvm::Value* rightExp = this->childPtr[2]->IRBuildExp();
            if (leftExp->getType() != theBuilder.getInt1Ty() || rightExp->getType() != theBuilder.getInt1Ty())
            {
                throw("Logic oper only for bool\n");
            }
            if (this->childPtr[1]->childPtr[0]->nodeName->compare("&&") == 0)
            {
                return theBuilder.CreateAnd(leftExp, rightExp, "andTmp");
            }
            else if (this->childPtr[1]->childPtr[0]->nodeName->compare("||") == 0)
            {
                return theBuilder.CreateOr(leftExp, rightExp, "orTmp");
            }
        }
        // + - * /
        else
        {
            llvm::Value* leftExp  = this->childPtr[0]->IRBuildExp();
            llvm::Value* rightExp = this->childPtr[2]->IRBuildExp();
            if (leftExp->getType() == theBuilder.getInt1Ty() || rightExp->getType() == theBuilder.getInt1Ty())
            {
                throw("+ - * / oper not for bool\n");
            }
            // 类型转换 都转换成float或者int
            if (leftExp->getType() != rightExp->getType())
            {
                if (leftExp->getType() == theBuilder.getFloatTy())
                {
                    rightExp = this->typeCast(rightExp, leftExp->getType());
                }
                else if (rightExp->getType() == theBuilder.getFloatTy())
                {
                    leftExp = this->typeCast(leftExp, rightExp->getType());
                }
                else if (leftExp->getType() == theBuilder.getInt32Ty())
                {
                    rightExp = this->typeCast(rightExp, leftExp->getType());
                }
                else if (rightExp->getType() == theBuilder.getInt32Ty())
                {
                    leftExp = this->typeCast(leftExp, rightExp->getType());
                }
            }
            if (this->childPtr[1]->childPtr[0]->nodeName->compare("+") == 0)
            {
                if (leftExp->getType() == theBuilder.getFloatTy())
                {
                    return theBuilder.CreateFAdd(leftExp, rightExp, "FAddTmp");
                }
                else
                {
                    return theBuilder.CreateAdd(leftExp, rightExp, "AddTmp");
                }
            }
            else if (this->childPtr[1]->childPtr[0]->nodeName->compare("-") == 0)
            {
                if (leftExp->getType() == theBuilder.getFloatTy())
                {
                    return theBuilder.CreateFSub(leftExp, rightExp, "FSubTmp");
                }
                else
                {
                    return theBuilder.CreateSub(leftExp, rightExp, "SubTmp");
                }
            }
            else if (this->childPtr[1]->childPtr[0]->nodeName->compare("*") == 0)
            {
                if (leftExp->getType() == theBuilder.getFloatTy())
                {
                    return theBuilder.CreateFMul(leftExp, rightExp, "FMulTmp");
                }
                else
                {
                    return theBuilder.CreateMul(leftExp, rightExp, "MulTmp");
                }
            }
            else if (this->childPtr[1]->childPtr[0]->nodeName->compare("/") == 0)
            {
                if (leftExp->getType() == theBuilder.getFloatTy())
                {
                    return theBuilder.CreateFDiv(leftExp, rightExp, "FDivTmp");
                }
                else
                {
                    return theBuilder.CreateSDiv(leftExp, rightExp, "SDivTmp");
                }
            }
        }
    }
    // exp → sgOper exp
    // sgOper → MINUS | NOT | PLUS
    if (this->childPtr[0]->nodeValue->compare("sgOper") == 0)
    {
        if (this->childPtr[0]->childPtr[0]->nodeValue->compare("MINUS") == 0)
        {
            llvm::Value* tmp = this->childPtr[1]->IRBuildExp();
            if (tmp->getType() == theBuilder.getInt1Ty())
            {
                throw("NOT not for bool\n");
            }
            return theBuilder.CreateNeg(tmp, "negTmp");
        }
        else if (this->childPtr[0]->childPtr[0]->nodeValue->compare("NOT") == 0)
        {
            llvm::Value* tmp = this->childPtr[1]->IRBuildExp();
            if (tmp->getType() != theBuilder.getInt1Ty())
            {
                throw("! only for bool\n");
            }
            return theBuilder.CreateNot(tmp, "notTmp");
        }
        else if (this->childPtr[0]->childPtr[0]->nodeValue->compare("PLUS") == 0)
        {
            return this->childPtr[1]->IRBuildExp();
        }
    }
    // exp → LPT exp RPT
    if (this->childPtr[0]->nodeValue->compare("LPT") == 0)
    {
        return this->childPtr[1]->IRBuildExp();
    }
    // exp → ID Array | ID funcCall
    if (this->childPtr[0]->nodeValue->compare("ID") == 0 && this->childNum == 2)
    {
        // exp → ID Array
        if (this->childPtr[1]->nodeValue->compare("Array") == 0)
        {
            if (this->childPtr[1]->childNum == 3)
            {
                llvm::Value* arrayPtr = this->IRBuildID();
                return theBuilder.CreateLoad(arrayPtr->getType()->getPointerElementType(), arrayPtr, "arrayTmp");
            }
            else
            {
                return this->IRBuildID();
            }
        }
        // exp → ID funcCall  ID()
        else if (this->childPtr[1]->nodeValue->compare("funcCall") == 0)
        {
            // funcCall → ()
#if DEBUG
            std::cout << "funcCall func()..." << std::endl;
#endif
            if (this->childPtr[1]->childNum == 2)
            {
                // Look up the name in the global module table.
                llvm::Function* calleeF = generator->gModule->getFunction(*this->childPtr[0]->nodeName);
                if (calleeF == nullptr)
                {
                    throw("Function Undeclared\n");
                }
                llvm::SmallVector<llvm::Value*, 2> indexVector;
                llvm::ArrayRef<llvm::Value*>       indexList(indexVector);
                return theBuilder.CreateCall(calleeF, indexList, "callTmp");
            }
            // funcCall → (argList)
            else if (this->childPtr[1]->childNum == 3)
            {
                // Look up the name in the global module table.
                if (this->childPtr[0]->nodeName->compare("print") == 0)
                {
                    return this->IRBuildPrint(false, false);
                }
                else if (this->childPtr[0]->nodeName->compare("println") == 0)
                {
                    return this->IRBuildPrint(true, false);
                }
                else if (this->childPtr[0]->nodeName->compare("printf") == 0)
                {
                    return this->IRBuildPrint(false, true);
                }
                else if (this->childPtr[0]->nodeName->compare("scan") == 0)
                {
                    return this->IRBuildScan();
                }

                llvm::Function* calleeF = generator->gModule->getFunction(*this->childPtr[0]->nodeName);
                if (calleeF == nullptr)
                {
                    throw("Function Undeclared\n");
                }
                std::vector<llvm::Value*>*   argsV = this->childPtr[1]->childPtr[1]->getArgList();
                llvm::ArrayRef<llvm::Value*> argsList(*argsV);
                return theBuilder.CreateCall(calleeF, argsList, "callTmp");
            }
        }
    }
    // expStmt → SEMICOLON
    return NULL;
}

/**
 * @description: if相关语句IR生成
 * @param
 * @return {*}
 * selecStmt → IF LPT exp RPT stmt %prec LOWER_THAN_ELSE | IF LPT exp RPT stmt ELSE stmt
 */
llvm::Value* astNode::IRBuildSelec()
{
#if DEBUG
    std::cout << "start if building..." << std::endl;
#endif
    llvm::Value*      thenV       = nullptr;
    llvm::Value*      elseV       = nullptr;
    llvm::Value*      condV       = theBuilder.CreateICmpEQ(this->childPtr[2]->IRBuildExp(), llvm::ConstantInt::get(theBuilder.getInt1Ty(), 1), "ifCond");
    llvm::Function*   theFunction = theBuilder.GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB      = llvm::BasicBlock::Create(theContext, "then", theFunction);   // 自动加到函数中
    llvm::BasicBlock* elseBB      = llvm::BasicBlock::Create(theContext, "else");
    llvm::BasicBlock* mergeBB     = llvm::BasicBlock::Create(theContext, "ifcond");
    llvm::BranchInst* select      = theBuilder.CreateCondBr(condV, thenBB, elseBB);   // 插入条件分支语句的指令
#if DEBUG
    std::cout << "start then building..." << std::endl;
#endif
    // Then语句处理
    theBuilder.SetInsertPoint(thenBB);
    thenV = this->childPtr[4]->IRBuildStmt();
    theBuilder.CreateBr(mergeBB);           // 插入跳转到Merge分支的指令
    thenBB = theBuilder.GetInsertBlock();   // 获取Then语句的出口
#if DEBUG
    std::cout << "then built" << std::endl;
#endif

    // Else语句处理
    theFunction->getBasicBlockList().push_back(elseBB);   // 添加到函数中去
    theBuilder.SetInsertPoint(elseBB);
    // selecStmt → IF LPT exp RPT stmt ELSE stmt
    if (this->childNum == 7)
    {
        elseV = this->childPtr[6]->IRBuildStmt();
    }
    theBuilder.CreateBr(mergeBB);           // 插入跳转到Merge分支的指令
    elseBB = theBuilder.GetInsertBlock();   // 获取Else语句的出口

    theFunction->getBasicBlockList().push_back(mergeBB);
    theBuilder.SetInsertPoint(mergeBB);

    return select;
}

/**
 * @description: while相关语句生成
 * @param
 * @return {*}
 * iterStmt → WHILE LPT exp RPT stmt
 */
llvm::Value* astNode::IRBuildIter()
{
    llvm::Function*   theFunction = theBuilder.GetInsertBlock()->getParent();
    llvm::BasicBlock* condBB      = llvm::BasicBlock::Create(theContext, "cond", theFunction);
    llvm::BasicBlock* bodyBB      = llvm::BasicBlock::Create(theContext, "body", theFunction);
    llvm::BasicBlock* endBB       = llvm::BasicBlock::Create(theContext, "end", theFunction);
    generator->blockStack.push(endBB);
    theBuilder.CreateBr(condBB);   // 跳转到条件分支

    // condBB基本块
    theBuilder.SetInsertPoint(condBB);
    llvm::Value*      condV  = theBuilder.CreateICmpEQ(this->childPtr[2]->IRBuildExp(), llvm::ConstantInt::get(theBuilder.getInt1Ty(), 1), "whileCond");
    llvm::BranchInst* select = theBuilder.CreateCondBr(condV, bodyBB, endBB);   // 插入分支语句的指令
    condBB                   = theBuilder.GetInsertBlock();                     // 获取条件语句的出口

    // bodyBB基本块
    theBuilder.SetInsertPoint(bodyBB);
    this->childPtr[4]->IRBuildStmt();
    // llvm::Value* bodyV = this->childPtr[4]->IRBuildStmt(func);
    // if (bodyV == nullptr)   // break;
    // {
    //     theBuilder.CreateBr(endBB);   // 跳转到endBB
    // }
    theBuilder.CreateBr(condBB);   // 跳转到condBB

    // endBB基本块
    theBuilder.SetInsertPoint(endBB);
    generator->blockStack.pop();
    return select;
}

/**
 * @description: return & break 语句相关IR生成
 * @param {llvm::Function*} func 当前所在函数
 * @return {*}
 * retStmt → RETURN exp SEMICOLON | RETURN SEMICOLON | BREAK SEMICOLON
 */
llvm::Value* astNode::IRBuildRet()
{
    // retStmt → RETURN exp SEMICOLON
    if (this->childNum == 3 && this->childPtr[0]->nodeValue->compare("RETURN") == 0)
    {
        llvm::Value* ret = this->childPtr[1]->IRBuildExp();
        return theBuilder.CreateRet(ret);
    }
    // retStmt → RETURN SEMICOLON
    else if (this->childPtr[0]->nodeValue->compare("RETURN") == 0 && this->childNum == 2)
    {
        return theBuilder.CreateRetVoid();
    }
    // retStmt → BREAK SEMICOLON
    else if (this->childPtr[0]->nodeValue->compare("BREAK") == 0)
    {
        return theBuilder.CreateBr(generator->blockStack.top());
    }
    else
    {
        throw("RET ERROR\n");
    }
}

/**
 * @description: Print IR生成
 * @param {*}
 * @return {*}
 * exp → ID funcCall
 * funcCall → LPT argList RPT | LPT RPT
 * argList → exp COMMA argList | exp
 */
llvm::Value* astNode::IRBuildPrint(bool isPrintln, bool isPrintf)
{
#if DEBUG
    std::cout << "print build start..." << std::endl;
#endif
    std::string                formatStr = "";
    std::vector<llvm::Value*>* printArgs = new std::vector<llvm::Value*>;
    astNode*                   node      = this->childPtr[1]->childPtr[1];
    while (true)
    {
        llvm::Value* tmp = node->childPtr[0]->IRBuildExp();
        if (tmp->getType() == theBuilder.getFloatTy())
        {
            tmp = theBuilder.CreateFPExt(tmp, theBuilder.getDoubleTy(), "tmpDouble");
        }
        if (node->childNum == 1)
        {
            printArgs->push_back(tmp);
            break;
        }
        else if (node->childNum == 3)
        {
            printArgs->push_back(tmp);
            node = node->childPtr[2];
        }
        else
        {
            throw("print arg error\n");
        }
    }
    if (isPrintf)
    {
        return theBuilder.CreateCall(generator->printf, *printArgs, "printf");
    }

    for (auto& argValue : *printArgs)
    {

        if (argValue->getType() == theBuilder.getInt32Ty())
        {
            formatStr += "%d";
        }
        else if (argValue->getType() == theBuilder.getInt8Ty())
        {
            formatStr += "%c";
        }
        else if (argValue->getType() == theBuilder.getInt1Ty())
        {
            formatStr += "%d";
        }
        else if (argValue->getType() == theBuilder.getDoubleTy())
        {
            formatStr += "%lf";
        }
        else if (argValue->getType() == theBuilder.getInt8PtrTy())
        {
            formatStr += "%s";
        }
        else if (argValue->getType()->getPointerElementType()->isArrayTy() && argValue->getType()->getPointerElementType()->getArrayElementType() == theBuilder.getInt8Ty())
        {
            formatStr += "%s";
            llvm::SmallVector<llvm::Value*, 2> indexVector;
            indexVector.push_back(theBuilder.getInt32(0));
            indexVector.push_back(theBuilder.getInt32(0));
            llvm::ArrayRef<llvm::Value*> indexList(indexVector);
            argValue = theBuilder.CreateGEP(argValue, indexList);
        }
        else
        {
            throw("Invalid type to write.");
        }
    }
    if (isPrintln)
    {
        formatStr += "\n";
    }
    auto            formatConst  = llvm::ConstantDataArray::getString(theContext, formatStr.c_str());
    auto            formatStrVar = new llvm::GlobalVariable(*(generator->gModule), llvm::ArrayType::get(theBuilder.getInt8Ty(), formatStr.size() + 1), true, llvm::GlobalValue::ExternalLinkage, formatConst, ".str");
    auto            zero         = llvm::Constant::getNullValue(theBuilder.getInt32Ty());
    llvm::Constant* indices[]    = {zero, zero};
    auto            varRef       = llvm::ConstantExpr::getGetElementPtr(formatStrVar->getType()->getElementType(), formatStrVar, indices);
    printArgs->insert(printArgs->begin(), varRef);
    return theBuilder.CreateCall(generator->printf, *printArgs, "printf");
}

/**
 * @description: Scan IR生成
 * @param {*}
 * @return {*}
 * exp → ID funcCall
 * funcCall → LPT argList RPT | LPT RPT
 * argList → exp COMMA argList | exp
 */
llvm::Value* astNode::IRBuildScan()
{
    // #if DEBUG
    //     std::cout << "Scan Build Start..." << std::endl;
    // #endif
    //     std::string                formatStr = "";
    //     std::vector<llvm::Value*>* scanArgs  = new std::vector<llvm::Value*>;
    //     astNode*                   node      = this->childPtr[1]->childPtr[1];
    //     while (true)
    //     {
    //         if (node->childNum == 1)
    //         {
    //             scanArgs->push_back(node->childPtr[0]->IRBuildID());
    //             break;
    //         }
    //         else
    //         {
    //             scanArgs->push_back(node->childPtr[0]->IRBuildID());
    //             node = node->childPtr[2];
    //         }
    //     }
    // #if DEBUG
    //     std::cout << "argList get..." << std::endl;
    // #endif
    //     for (auto argValue : *scanArgs)
    //     {
    //         if (argValue->getType()->getPointerElementType() == theBuilder.getInt32Ty())
    //         {
    //             formatStr += "%d";
    //         }
    //         else if (argValue->getType()->getPointerElementType() == theBuilder.getInt8Ty())
    //         {
    //             formatStr += "%c";
    //         }
    //         else if (argValue->getType()->getPointerElementType() == theBuilder.getInt1Ty())
    //         {
    //             formatStr += "%d";
    //         }
    //         else if (argValue->getType()->getPointerElementType() == theBuilder.getFloatTy())
    //         {
    //             formatStr += "%lf";
    //         }
    //         else if (argValue->getType()->getPointerElementType()->isArrayTy() && argValue->getType()->getPointerElementType()->getArrayElementType() == theBuilder.getInt8Ty())
    //         {
    //             formatStr += "%s";
    //         }
    //         else
    //         {
    //             throw("Invalid type to read.\n");
    //         }
    //     }
    //     scanArgs->insert(scanArgs->begin(), theBuilder.CreateGlobalStringPtr(formatStr));
    //     llvm::ArrayRef<llvm::Value*> argList(*scanArgs);
    // #if DEBUG
    //     std::cout << "Scan Build ok..." << std::endl;
    // #endif
    //     return theBuilder.CreateCall(generator->scanf, argList, "scanf");
}

/**
 * @description: 寻找ID的内存地址
 * @param {llvm::Function*} func
 * @return {*}
 * exp → ID | ID Array | ID funcCall
 * Array → LSB exp RSB | LSB RSB
 */
llvm::Value* astNode::IRBuildID()
{
    //查找局部变量
    llvm::Value* var = theBuilder.GetInsertBlock()->getParent()->getValueSymbolTable()->lookup(*this->childPtr[0]->nodeName);
    if (var == nullptr)
    {
        // 查找全局变量
        var = generator->gModule->getGlobalVariable(*this->childPtr[0]->nodeName, true);
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
                llvm::Value*                       index  = this->childPtr[1]->childPtr[1]->IRBuildExp();
                llvm::Value*                       const0 = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(theContext), 0);
                llvm::SmallVector<llvm::Value*, 2> indexVector;
                indexVector.push_back(const0);
                indexVector.push_back(index);
                llvm::ArrayRef<llvm::Value*> indexList(indexVector);
                llvm::Value*                 arrayPtr = theBuilder.CreateGEP(var, indexList, "tmpArry");
                return arrayPtr;
            }
            else if (this->childPtr[1]->childNum == 2)
            {
                return var;
            }
        }
        else
        {
            throw("Func call cannot be leftvalue in exp\n");
        }
    }
    // exp → ID
    else if (this->childNum == 1)
    {
        return var;
    }
    return NULL;
}

/**
 * @description: 二元运算隐式类型转换
 * @param {llvm::Value*} elem1 待类型转换元素
 * @param {llvm::Type*} type2 目标类型
 * @return {*}
 */
llvm::Value* astNode::typeCast(llvm::Value* elem1, llvm::Type* type2)
{
    llvm::Type*                type1 = elem1->getType();
    llvm::Instruction::CastOps op;
    // int -> char
    if (type1 == theBuilder.getInt32Ty() && type2 == theBuilder.getInt8Ty())
    {
        op = llvm::Instruction::CastOps::Trunc;
    }
    // int -> float
    else if (type1 == theBuilder.getInt32Ty() && type2 == theBuilder.getFloatTy())
    {
        op = llvm::Instruction::CastOps::SIToFP;
    }
    // char -> int
    else if (type1 == theBuilder.getInt8Ty() && type2 == theBuilder.getInt32Ty())
    {
        op = llvm::Instruction::CastOps::ZExt;
    }
    // char -> float
    else if (type1 == theBuilder.getInt8Ty() && type2 == theBuilder.getFloatTy())
    {
        op = llvm::Instruction::CastOps::UIToFP;
    }
    // float -> int
    else if (type1 == theBuilder.getFloatTy() && type2 == theBuilder.getInt32Ty())
    {
        op = llvm::Instruction::CastOps::FPToSI;
    }
    // float -> char
    else if (type1 == theBuilder.getFloatTy() && type2 == theBuilder.getInt8Ty())
    {
        op = llvm::Instruction::CastOps::FPToUI;
    }
    else
    {
        throw("TypeCast Failed\n");
    }
    return theBuilder.CreateCast(op, elem1, type2, "typeCast");
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
    return tmpB.CreateAlloca(type, nullptr, varName);
}
