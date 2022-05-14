## myCompiler
a simple Cminus compiler based on Cpp

### 0. 测试环境

Unix/Linux macOS

flex 2.6.4

bison (GNU Bison) 3.8.2

llvm 13.0.1

### 1. 文件组织结构

```apl
myCompiler
├── src
│   ├── cmm.l
│   ├── cmm.y
│   ├── 
│   ├── 
│   │ 
│   │ 
│   ├── 
│   │     
│   └── Makefile      
│          
│       
│      
│       
│   
├── test
│ 
└── vis
    ├──
    ├──
    └──
 
```



### 2. 相关参考 & 代码规范

[Lex&Yacc 联合原理](https://zhuanlan.zhihu.com/p/143867739)

[可变参数函数](https://songlee24.github.io/2014/07/22/cpp-changeable-parameter/)

[LLVM官方文档](https://llvm.org/doxygen/namespacellvm.html)

[LLVM教程](https://llvm-tutorial-cn.readthedocs.io/en/latest/chapter-1.html)

[LLVM IR相关语法与接口](https://blog.csdn.net/qq_42570601/category_10200372.html)

[LLVM编译器架构](https://blog.csdn.net/xfxyy_sxfancy/category_9264747.html?spm=1001.2014.3001.5482)

[LLVM例程](https://cloud.tencent.com/developer/article/1879336?from=article.detail.1352547)

[LLVM中文](https://llvm.liuxfe.com/tutorial/langimpl/)

[IRBuilderBase API](https://llvm.org/doxygen/classllvm_1_1IRBuilderBase.html#a06d64f226fabd94d694d6abd82e3c7b3)

**规范：**

变量定义采用小驼峰

不使用`using namespace std`，使用std库时手动添加`std::`

### 3. 词法分析模块

词法分析负责ast的leaf节点构建（将每个终结符作为leaf节点），语法分析负责剩余节点构建

### 4. 语法分析模块

#### a. cmm.y

**定义的BNF语法**

全大写表示终结符，采用自顶向下分析

左递归化为右递归

```yacas
program → decList
decList → dec decList | %empty
dec → varDeclaration | funcDeclaration 🍺

varDeclaration → typeSpecifier varDecList SEMICOLON
typeSpecifier → TYPE
varDecList → varDef | varDef COMMA varDecList
varDef → ID | ID LSB INT RSB

funcDeclaration → typeSpecifier funcDec compoundStmt 🍺 // 还差函数的入/出栈
funcDec → ID LPT paramList RPT | ID LPT RPT 🍺
paramList → paramDec COMMA paramList | paramDec 🍺
paramDec → typeSpecifier paramDef 🍺
paramDef → ID | ID LSB RSB 🍺
compoundStmt → LCB content RCB
content → localDec stmtList | %empty
localDec → varDeclaration localDec | %empty
stmtList → stmt stmtList | %empty
stmt → expStmt | compoundStmt | selecStmt | iterStmt | retStmt
selecStmt → IF LPT exp RPT stmt | IF LPT exp RPT stmt ELSE stmt
iterStmt → WHILE LPT exp RPT stmt
retStmt → RETURN exp SEMICOLON | RETURN SEMICOLON | BREAK SEMICOLON
expStmt → exp SEMICOLON | SEMICOLON
exp → exp dbOper exp | sgOper exp | LPT exp RPT | ID Array | ID funcCall | sgFactor
dbOper → PLUS | MINUS | MULTI | DIV | MOD | RELOP | ASSIGN | AND | OR
sgOper → MINUS | NOT | PLUS
Array → LSB exp RSB | LSB RSB
funcCall → LPT argList RPT | LPT RPT
argList → exp COMMA argList | exp
sgFactor → ID | INT | FLOAT | CHAR | BOOLEAN | STR
```

#### b. AST

可以有两种设计：

- 将ast设计为左指针指向子节点，右指针指向兄弟节点
- 每个节点保存子节点个数和子节点指针数组

我认为第二种比较容易实现和操作，因此在节点类中定义保存该节点的子节点指针的vector

```cpp
class astNode
{
private:
    std::string *nodeName;           // 存放节点的name
    std::string *nodeValue;          // 存放节点的value
    std::vector<astNode *> childPtr; // 存放子节点指针的vector
    int childNum;                    // 子节点个数
    int lineIndex;                   // 该节点对应的终结符/非终结符所在的行号

public:
    astNode(char *nodeName, std::string nodeValue, int lineIndex);          // yytext存储类型为char*，用作词法分析中的leaf构建
    astNode(std::string nodeName, std::string nodeValue, int childNum, ...); //可变参数，用作语法分析中的其余节点构建
    ~astNode();
    //相关值的method以及llvm提供的c接口
};
```

关于可变参数，C++11提供了两种主要的方法：如果所有的实参类型相同，可以传递一个名为**initializer_list**的标准库类型；如果实参的类型不同，则编写**可变参数模板**。

### 5. 语义分析模块 IR

#### 5.0 LLVM

```yacas
Source Code -> | Frontend | Optimizer | Backend | -> Machine Code
```

LLVM的一大特色就是有着独立的、完善的、严格约束的中间代码表示。这种中间代码，就是LLVM的字节码前端生成这种中间代码，后端自动进行各类优化分析。

#### 5.1 运行时环境

```cpp
// 记录了LLVM的核心数据结构，比如类型和常量表，不过不太需要关心它的内部
llvm::LLVMContext gLLVMContext;
// 用于创建LLVM指令
llvm::IRBuilder<> gIRBuilder(gLLVMContext);
```

#### 5.2 符号表

**⚠️如何判断全局变量与局部变量？---------------------待写**

##### 5.2.1 全局变量

[How to create GlobalVar - Stackoverflow](https://stackoverflow.com/questions/7787308/how-can-i-declare-a-global-variable-in-llvm)

```c++
// reference 1
// 将全局变量插入全局变量表 
// This creates a global and inserts it before the specified other global.
llvm::GlobalVariable::GlobalVariable(module, type, isConstant, linkage, initializer, name, nullptr, NotThreadLocal, false)
 
// reference 2
// Global Variable Declarations
GlobalVariable* gvar_ptr_abc = new GlobalVariable(/*Module=*/*mod, 
        /*Type=*/PointerTy_0,
        /*isConstant=*/false,
        /*Linkage=*/GlobalValue::CommonLinkage,
        /*Initializer=*/0, // has initializer, specified below
        /*Name=*/"abc");
gvar_ptr_abc->setAlignment(4);
// Constant Definitions
ConstantPointerNull* const_ptr_2 = ConstantPointerNull::get(PointerTy_0);
// Global Variable Definitions
gvar_ptr_abc->setInitializer(const_ptr_2);
```

**⚠️另外需要判断全局变量是否重复定义------------------待写**

` llvm::getGlobalVariable(name) `在全局变量表中查找全局变量

##### 5.2.2 局部变量

插入对应函数的符号表

#### 5.3 类型处理 🍺

`def.h`中宏定义类型

`astNode::getNodeType()`通过扫描typeSpecifier将类型分为 int float char bool，返回对应的宏

`astNode::getLLvmType()`通过`getValueType`的返回值得到`llvm::Value`

```c++
IntegerType type_i32 = Type::getInt32Ty(context);	//int
//int,使用IntegerType的get方法和Type的getInt32Ty方法是一样的，我一般用第一种，感觉方便
IntegerType type_i32 = IntegerType::get(context, 32);	

Type::getInt64Ty(context);	//long
Type::getFloatTy(context);	//float
Type::getDoubleTy(context);	//double
Type::getInt8Ty(context);	//char
Type::getVoidTy(context);	//void
```

指针类型

```c++
//int* int&，int指针和引用，其他类型都一样，改变Type::getInt32Ty(context)为预期类型即可
PointerType* int_pointer = Type::getInt32PtrTy(context);
PointerType* int_pointer = PointerType::get(Type::getInt32Ty(context), 0);	

//char*、void*对应的类型都是i8*
PointerType* char_pointer = PointerType::get(IntegerType::get(mod->getContext(), 8), 0);
```

数组类型

```c++
//长度为4的整型数组，int[4]
ArrayType* array_type = ArrayType::get(Type::getInt32Ty(context), 4);
```

#### 5.4 类型转换

[IR类型转换](https://zhuanlan.zhihu.com/p/163063995)

`CreateCast`



#### 5.5 变量处理

类型 变量名

`varList`保存变量名和类型（var or array，arraysize=0则为ptr）

判断是否为全局变量，插入符号表。

#### 5.6 函数处理

[LLVM提供的C接口](https://blog.csdn.net/qq_42570601/article/details/108059403)

定义一个函数的步骤可以简单分为五步：返回值类型 - 参数类型 - 前两者构成函数类型 - 根据函数类型声明函数 - 获取函数参数(如果有参数)，定义函数体

```cpp
//构建函数类型
llvm::FunctionType *type = llvm::FunctionType::get(returnType, functionArgs, /*isVarArg*/ false);
//构建函数 Creates a new function and attaches it to a module.
llvm::Function* F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, Module;
                                           
llvm::BasicBlock* entry = llvm::BasicBlock::Create(LLVMContext, "entry", func);
//created instructions should be appended to the end of the specified block.
Builder.SetInsertPoint(entry);
```



#### 5.7 分支&循环

[分支&循环](https://blog.csdn.net/qq_42570601/article/details/107771289)

##### 5.7.1 if else

#### 5.8 表达式
