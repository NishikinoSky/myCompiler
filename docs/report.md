## 1. 项目介绍

### 1.1 简介

本项目基于C++17实现了一个Cminus编译器。词法分析基于flex，语法分析基于bison，两者生成抽象语法树（Abstract Syntax Tree，AST）后调用LLVM IR提供的C++ API生成LLVM中间代码，再调用LLVM后端（backend）生成可执行文件。

同时，本项目基于HTML+CSS+JavaScript实现了一个简单的web IDE，可以进行基本的代码书写、编译运行、文件打开与保存。

### 1.2 文件组织结构

```yacas
myCompiler
├── docs
│		└── report.pdf
├── src
│   ├── tokenGen.l
│   ├── gramGen.y
│   ├── ast.h
│   ├── ast.cpp
│   ├── codeGen.h 
│   ├── codeGen.cpp 
│   ├── def.h
│   ├── main.cpp  
│   └── Makefile       
├── front
│		├── nodeModules
│		├── images
│		├── css
│		├── js
│		└── html
└── test
		└── test.cmm
```

### 1.3 运行环境

操作系统：macOS12.4 arm64

编译器：

- Flex 2.6.4
- Bison (GNU Bison) 3.8.2
- LLVM 13.0.1
- Clang 13.1.6

### 1.4 Cminus介绍

本项目可以编译的语言为Cminus。

支持的数据类型：int、float、char、bool 以及对应类型的数组。 

支持的语法类型：各类型全局/局部变量的声明与赋值，函数的声明与调用，各类型变量的计算，条件语句if-else，循环语句while&break，指定数据输出，注释

## 2. 词法分析模块

### 2.1 模块介绍

本模块主要使用lex解析提取Cminus代码中的终结符（token）并且负责AST的叶节点（leaf）的构建。对应文件为`tokenGen.l`，编译后生成`tokenGen.cpp`。

### 2.2 Lex

#### 2.2.1 Lex介绍

Lex是一个产生词法分析器（lexical analyzer）的程序。常常与yacc 语法分析器产生程序（parser generator）一起使用。

Lex读进一个代表词法分析器规则的输入字符串流，然后输出以C语言实做的词法分析器源代码。

编译方式：

```bash
flex -o tokenGen.cpp tokenGen.l
```

Lex文件主要构成如下：

```less
{definitions}
%%
{rules}
%%
{code}
```

#### 2.2.2 定义区（Definitions）

```less
%{
    #include "ast.h" //ast头文件
    #include "gramGen.hpp" //语法分析生成的头文件
    #include <string.h>
%}
```

#### 2.2.3 规则区（Rules）

token声明如下：

```less
/*key words*/
ELSE else
IF if
RETURN return
WHILE while
BREAK break
TYPE "int"|"float"|"char"|"bool"|"void"
BOOL "true"|"false"

/*identifiers*/
digit [0-9]
letter [_a-zA-Z]
ids [_0-9a-zA-Z]
INT 0|[1-9]{digit}*
FLOAT {digit}+\.{digit}+
CHAR \'.\'|\'\\.\'
STR \"(\\.|[^"\\])*\"
ID {letter}+{ids}*
COMMA \,
SEMICOLON \;
NEWLINE \n
TAB \t
CR \r
BLANK " "

/*operators*/
PLUS \+
MINUS \-
MULTI \*
DIV \/
RELOP "<"|"<="|">"|">="|"=="|"!="
ASSIGN "="
LOGIC "&&"|"||"
NOT !
LPT \(
RPT \)
LSB \[
RSB \]
LCB \{
RCB \}

/*comments*/
COMMENT \/\/[^\n]*|"/*"([^\*]|(\*)*[^\*/])*(\*)*"*/"
```

匹配到符合规则的文本后调用`ast.h`中定义的`astNode`类叶节点的构造函数`astNode(char* nodeName, std::string nodeValue)`，并将终结符返回给`gramGen.y`进行语法分析。该类将在 **3.3 抽象语法树 AST** 部分详细介绍。

以token `INT 0|[1-9]{digit}*` 为例：

```less
{INT}   {yylval.asTree = new astNode(yytext, "INT"); return INT;}
```

#### 2.2.4 代码区（Code）

`yywrap()`为Flex在翻译为C文件时会产生的自定义函数，在文件末尾调用。若返回值为1，则停止解析，由此可以实现多文件的解析。

```c++
int yywrap(void) {
    return 1;
}
```

## 3. 语法分析模块

### 3.1 模块介绍

本模块主要根据定义的语法规则利用Yacc对AST叶节点之外的节点进行构建，结合词法分析生成整棵AST。

对应文件为`gramGen.y` `ast.h` `ast.cpp`。

### 3.2 Yacc

#### 3.2.1 Yacc介绍

Yacc（Yet Another Compiler Compiler），是一个用来生成编译器的编译器（编译器代码生成器）。Yacc生成的编译器主要是用C语言写成的语法解析器（Parser），需要与词法解析器Lex一起使用，再把两部分产生出来的C程序一并编译。

yacc的输入是巴科斯范式（BNF）表达的语法规则以及语法规约的处理代码，输出的是基于表驱动的编译器，包含输入的语法规约的处理代码部分。对应文件为`gramGen.y`，编译后生成`gramGen.hpp`和`gramGen.cpp`。

编译方式：

```bash
bison -d -o gramGen.cpp gramGen.y
```

yacc文件主要构成如下：

```yacas
definitions
%%
rules
%%
code
```

#### 3.2.2 定义区（Definitions）

```c
%{
    #include <stdio.h>
    #include "ast.h"
    int yylex(void);
    void yyerror(char *);
    extern astNode* astRoot;
%}

%union
{
    class astNode* asTree;
}

/*terminals*/
%token <asTree> ELSE
%token <asTree> IF
%token <asTree> RETURN
%token <asTree> WHILE
%token <asTree> BREAK
%token <asTree> TYPE
%token <asTree> BOOL

%token <asTree> INT
%token <asTree> FLOAT
%token <asTree> CHAR
%token <asTree> STR
%token <asTree> ID
%token <asTree> COMMA
%token <asTree> SEMICOLON

%token <asTree> PLUS
%token <asTree> MINUS
%token <asTree> MULTI
%token <asTree> DIV
%token <asTree> RELOP
%token <asTree> ASSIGN
%token <asTree> LOGIC
%token <asTree> NOT
%token <asTree> LPT
%token <asTree> RPT
%token <asTree> LSB
%token <asTree> RSB
%token <asTree> LCB
%token <asTree> RCB

%type <asTree> program
%type <asTree> decList
%type <asTree> dec
%type <asTree> varDeclaration
%type <asTree> funcDeclaration
%type <asTree> typeSpecifier
%type <asTree> varDecList
%type <asTree> varDef
%type <asTree> funcDec
%type <asTree> compoundStmt
%type <asTree> paramList
%type <asTree> paramDec
%type <asTree> localDec
%type <asTree> stmtList
%type <asTree> stmt
%type <asTree> expStmt
%type <asTree> selecStmt
%type <asTree> iterStmt
%type <asTree> retStmt
%type <asTree> exp
%type <asTree> dbOper
%type <asTree> sgOper
%type <asTree> Array
%type <asTree> funcCall
%type <asTree> argList
%type <asTree> sgFactor

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%left PLUS MINUS MULTI DIV
%left RELOP LOGIC
%left LPT RPT LSB RSB
%right NOT ASSIGN
```

通过`union`将`class astNode*`类型表示为`astTree`，以便声明节点类型。

#### 3.2.3 规则区（Rules）

本项目定义的Cminus BNF采用自顶向下分析，并消除左递归。（全大写表示终结符）

```yacas
program → decList
decList → dec decList | %empty
dec → varDeclaration | funcDeclaration

varDeclaration → typeSpecifier varDecList SEMICOLON
typeSpecifier → TYPE
varDecList → varDef | varDef COMMA varDecList
varDef → ID | ID LSB INT RSB | ID LSB RSB

funcDeclaration → typeSpecifier funcDec compoundStmt
funcDec → ID LPT paramList RPT | ID LPT RPT
paramList → paramDec COMMA paramList | paramDec  
paramDec → typeSpecifier varDef  
compoundStmt → LCB localDec stmtList RCB  
localDec → varDeclaration localDec | %empty  
stmtList → stmt stmtList | %empty  
stmt → expStmt | compoundStmt | selecStmt | iterStmt | retStmt  
selecStmt → IF LPT exp RPT stmt %prec LOWER_THAN_ELSE | IF LPT exp RPT stmt ELSE stmt  
iterStmt → WHILE LPT exp RPT stmt  
retStmt → RETURN exp SEMICOLON | RETURN SEMICOLON | BREAK SEMICOLON  
expStmt → exp SEMICOLON | SEMICOLON
exp → exp dbOper exp | sgOper exp | LPT exp RPT | ID | ID Array | ID funcCall | sgFactor
dbOper → PLUS | MINUS | MULTI | DIV | RELOP | ASSIGN | AND | OR
sgOper → MINUS | NOT | PLUS
Array → LSB exp RSB | LSB RSB
funcCall → LPT argList RPT | LPT RPT
argList → exp COMMA argList | exp  
sgFactor → INT | FLOAT | CHAR | BOOL | STR
```

匹配到符合规则的语法后调用`ast.h`中定义的`astNode`类构造函数`astNode(std::string nodeName, std::string nodeValue, int childNum, ...)`构建非叶节点。该类将在 **3.3 抽象语法树 AST** 部分详细介绍。

以语法`dec → varDeclaration | funcDeclaration`为例：

```c++
dec:
    varDeclaration {
        $$ = new astNode("", "dec", 1, $1);
    }
    | funcDeclaration {
        $$ = new astNode("", "dec", 1, $1);
    };
```

#### 3.2.4 代码区（Code）

```c
void yyerror(char *str)
{
    fprintf(stderr,"error: %s\n",str);
}
```

### 3.3 抽象语法树 AST

在计算机科学中，抽象语法树（Abstract Syntax Tree），AST是源代码语法结构的一种抽象表示。它以树状的形式表现编程语言的语法结构，树上的每个节点都表示源代码中的一种结构。

本项目将语法树的构建和语义分析相关接口都封装在`astNode`类中，以便于直接调用LLVM C++API。

`astNode`类中语法分析相关成员：

```c++
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
};
```

- `std::string* nodeName`保存叶节点的值，也就是词法分析扫描到的输入文本单元。
- `std::string* nodeValue`保存flex中定义的token类型以及bison中定义的非终结符类型。
- ` std::vector<astNode*> childPtr`保存指向子节点的指针。

- `int childNum`保存子节点数量。

由于终结符和非终结符的节点构建方式不同，构造函数需要重载。

- `astNode(char* nodeName, std::string nodeValue)`负责AST叶节点（终结符）的构建。

  - `nodeName`可以通过flex的自定义全局变量`yytext`传入。`yytext`以`char*`类型存储匹配到的文本。
  - `nodeValue`直接将定义的token类型以字符串形式传入。

- ` astNode(std::string nodeName, std::string nodeValue, int childNum, ...)`负责AST非叶节点（非终结符）的构建。

  - 由于是非终结符，`nodeName`直接传入空字符串`""`。

  - `nodeValue`直接将定义的非终结符类型以字符串形式传入。

  - 每个非终结符子节点个数不一定相同但类型相同，可以采用省略符形参来实现可变参数函数。

    省略符形参的可变参数函数相关：

    ```c++
    #include<cstdarg>
    // va_list是一种数据类型，args用于持有可变参数。
    // 定义typedef char* va_list;
    va_list args;
    // 调用va_start并传入两个参数：第一个参数为va_list类型的变量，第二个参数为"..."前最后一个参数名
    // 将args初始化为指向可变参数列表的第一个参数
    va_start(args, paramN);
    // 检索参数，va_arg的第一个参数是va_list变量，第二个参数指定返回值的类型
    // 每一次调用va_arg会获取当前的参数，并自动更新指向下一个可变参数。
    va_arg(args,type);
    // 释放va_list变量
    va_end(args);
    ```

由此，在flex和bison中扫描到终结符和非终结符后分别调用对应的AST构造函数，即可生成整棵AST。

## 4. 语义分析模块

本模块通过自顶向下遍历AST进行语义分析，并调用LLVM提供的C++API生成LLVM IR（中间表示）。

### 4.1 LLVM

#### 4.1.1 LLVM & LLVM IR介绍

LLVM是一套编译器基础设施项目，包含一系列模块化的编译器组件和工具链，用来开发编译器前端和后端。

传统的静态编译器分为三个阶段：前端、优化和后端。LLVM的三阶段设计如下：

<img src="/Users/skyyyy/Documents/2022_SPRSUM/compiler/myCompiler/docs/report.assets/截屏2022-05-27 13.38.22.png" alt="截屏2022-05-27 13.38.22" style="zoom:50%;" />

其优点是如果需要支持一种新的编程语言，只需要实现一种新的前端。如果需要支持一种新的硬件设备，只需要实现一个新的后端。而优化阶段因为是针对了统一的LLVM IR，所以它是一个通用的阶段，不论是支持新的编程语言，还是支持新的硬件设备，这里都不需要对优化阶段做修改。所以从这里可以看出LLVM IR的作用。

LLVM IR主要有三种格式：一种是在内存中的编译中间语言，如BasicBlock，Instruction这种cpp类；一种是硬盘上存储的二进制中间语言（以.bc结尾），最后一种是可读的中间格式（以.ll结尾）。这三种中间格式是完全相等的。

#### 4.1.2 内存中的IR模型

直白而言，内存中的IR模型是一些cpp的类定义。

LLVM的基本组成为以下四类：

- Module类，Module可以理解为一个完整的编译单元。一般来说，这个编译单元就是一个源码文件，如一个后缀为cpp的源文件，有函数列表和全局变量表。
- Function类，对应于一个函数单元。Function可以描述两种情况，分别是函数定义和函数声明。
- BasicBlock类，表示一个基本代码块，“基本代码块”就是一段没有控制流逻辑的基本流程，相当于程序流程图中的基本过程（矩形表示）。
- Instruction类，指令类就是LLVM中定义的基本操作，比如加减乘除这种算数指令、函数调用指令、跳转指令、返回指令等等。

它们之间的关系如下：

<img src="/Users/skyyyy/Documents/2022_SPRSUM/compiler/myCompiler/docs/report.assets/截屏2022-05-27 17.12.22.png" alt="截屏2022-05-27 17.12.22" style="zoom:50%;" />

此外基本类还有Value类和User类，其中Value为各类型值的基类，函数、常量、变量、表达式等都可以转换为Value类型。

### 4.2 LLVM IR实现

本项目将LLVM IR相关函数封装在`astNode`类中，以便自顶向下遍历AST节点时生成对应的IR。同时在`codeGen.h`和`codeGen`类中声明生成LLVM IR所需的数据结构并控制LLVM IR的生成。

`astNode`类中LLVM IR实现相关成员：

```c++
class astNode
{
public:
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
```

`codeGen.h`中LLVM IR相关：

```c++
// 记录了LLVM的核心数据结构，比如类型和常量表，不过不太需要关心它的内部
static llvm::LLVMContext theContext;
// 用于创建LLVM指令
static llvm::IRBuilder<> Builder(theContext);

class codeGen
{
public:
    // 用于管理函数和全局变量，类似于类c++的编译单元(单个cpp文件)
    llvm::Module* gModule;
    // 块栈，用于多重循环break
    std::stack<llvm::BasicBlock*> blockStack;
    llvm::Function*               printf;
    llvm::Function*               printGen();
    void                          codeGenerator(astNode* root);
    codeGen();
    ~codeGen();
};
```

### 4.3 运行时环境

创建`LLVMContext`对象：

```c++
static llvm::LLVMContext theContext;
```

- `LLVMContext`（上下文类）是LLVM API中的一个不透明类，记录了LLVM的核心数据结构，保存上下文符号。内存IR模型中的每个LLVM实体（Modules、Values、Types、Constants等）都属于`LLVMContext`，不过不太需要关心它的内部。从概念上讲，`LLVMContext`提供隔离，不同上下文中的实体不能交互，由此实现多线程安全编译。

在`LLVMContext`中创建一个`Module`：

```c++
llvm::Module* gModule = new llvm::Module("myCMM", theContext);
```

创建LLVM指令：

```c++
static llvm::IRBuilder<> theBuilder(theContext);
```

### 4.4 IR生成入口

`llvm::Value* astNode::IRBuilder()`为IR生成的入口函数，负责处理以下语法：

```yacas
program → decList
decList → dec decList | %empty
dec → varDeclaration | funcDeclaration
```

当前节点的`nodeValue`为`dec`时，根据子节点的`nodeValue`调用不同的IRBuild函数。注意此处的`varDeclaration`事实上是全局变量的声明，因此在这里调用`IRBuildVar(bool isGlobal)`时需要传入参数`true`。

### 4.5 类型构建

#### 4.5.1 基本类型构建

在`def.h`中定义表示类型的宏：

```c++
#define ERROR -1
#define VAR 0
#define VAR_INT 1
#define VAR_FLOAT 2
#define VAR_CHAR 3
#define VAR_BOOL 4
#define VAR_VOID 5
#define ARRAY 6
```

数组和非数组的基本类型都为`int` `float` `char` `bool`，它们可以共用这四个类型定义，在此基础上再增加`ARRAY`来区分数组和非数组。

类型相关的API为：

- `int astNode::getNodeType(astNode* node)`：传入节点为`typeSpecifier`时，根据其`nodeName`返回对应的类型。
- `llvm::Type* astNode::getLLVMType(int type, int size, bool isArray)`：根据参数返回对应的LLVM类型。
  - 对于`int a`，则根据其类型`int`直接返回`Builder.getInt32Ty()`。
  - 对于`int a[]`，将其作为指针处理，返回`llvm::Type::getInt32PtrTy()`。
  - 对于`int a[size]`，将其作为数组处理，返回`llvm::ArrayType::get(theBuilder.getInt32Ty(), size)`

#### 4.5.2 类型转换

在表达式（二元表达式）操作中，左值和右值的数据类型需要统一，因此设计隐式类型转换。

`llvm::Value* astNode::typeCast(llvm::Value* elem1, llvm::Type* type2)`负责进行类型转换，`elem1`为待类型转换元素，`type2`为需要转换成的目标类型。

主要调用API `llvm::Value* llvm::IRBuilderBase::CreateCast(llvm::Instruction::CastOps op, llvm::Value* V, llvm::Type* DesTy, name)`来实现类型转换。

类型转换操作为`int` `float` `char`两两之间的转换共6种。

### 4.6 变量构建

#### 4.6.1 变量获取

,`std::vector<std::pair<int, std::string>>* astNode::getVarList()`为变量的获取，负责处理以下语法：

```yacas
varDecList → varDef | varDef COMMA varDecList
varDef → ID | ID LSB INT RSB | ID LSB RSB
```

每个变量的类型（VAR或者ARRAY）和变量名（终结符ID）作为一个pair，保存在`varListPtr`指向的vector中。若当前节点为`varDecList`则通过循环保存变量直到`varDecList`只有一个子节点`varDef`，至此所有变量获取完毕。

由于类型处理和变量声明中需要用到数组的大小，当获取数组变量时对其类型传入`ARRAY + arraySize`，需要使用时减去`ARRAY`则可获得数组大小。

#### 4.6.2 变量声明

`llvm::Value* astNode::IRBuildVar(bool isGlobal)`为变量声明相关的IR生成，将变量插入符号表。其负责处理以下语法：

```yacas
varDeclaration → typeSpecifier varDecList SEMICOLON
typeSpecifier → TYPE
```

首先调用`getVarList()`得到变量，再通过`getNodeType()`和`getLLVMType()`得到变量的LLVM类型。

变量分为全局变量和局部变量，两者构建方式不同。

##### 4.6.2.1 全局变量声明

首先需要判断一个变量是否为全局变量，给`IRBuildVar()`增加一个参数`bool isGlobal`，根据调用的位置传入符合的`isGlobal`即可。

再判断变量名是否重复，可利用API `llvm::Module::getGlobalVariable(string name, bool AllowInternal)`判断全局变量名是否在全局变量表中已经出现过。

将全局变量插入全局变量表需要以下API：

```c++
llvm::GlobalVariable* globalPtr = new GlobalVariable(/*Module=*/*module, 
        /*Type=*/llvmType,
        /*isConstant=*/false,
        /*Linkage=*/GlobalValue::CommonLinkage,
        /*Initializer=*/0, // has initializer, specified below
        /*Name=*/"");
```

按照IR语法需要对全局变量进行初始化，这里都初始化为0。

- 非数组：

  ```c++
  llvm::Constant* con = llvm::ConstantInt::get(llvmType, 0);
  gVarPtr->setInitializer(con);   //对全局变量初始化
  ```

- 数组：

  ```c++
  std::vector<llvm::Constant*> constArrayElems;
  llvm::Constant* con = llvm::ConstantInt::get(llvmType->getArrayElementType(), 0);   // 数组单个元素的类型，并且元素初始化为0
  for (int i = 0; i < arraySize; i++)
  {
  	constArrayElems.push_back(con);
  }
  llvm::ArrayType* arrayType  = llvm::ArrayType::get(llvmType->getArrayElementType(), arraySize);
  llvm::Constant*  constArray = llvm::ConstantArray::get(arrayType, constArrayElems);   //数组常量
  gArrayPtr->setInitializer(constArray);              //将数组常量初始化给全局常量       
  ```

  全局数组是以全局数组常量形式保存的。

##### 4.6.2.2 局部变量声明

为了能在符号表中查找局部变量，需要获得每个局部变量所属的函数，使用`theBuilder.GetInsertBlock()->getParent()`可以获得当前块所属的函数指针。再通过`llvm::Function::getValueSymbolTable()`获取当前函数的符号表指针，再通过 `llvm::ValueSymbolTable::lookup(string name)`查找局部变量名是否已经存在于符号表中即可判断局部变量是否重名。

为了创建局部变量，特别声明了一个辅助函数`llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* func, const std::string& varName, llvm::Type* type)`。该函数创建了一个IRBuilder对象`tmpB`，该对象指向Blockentry 的第一条指令(.Begin())，然后创建一个具有预期名称的alloca并返回它。

### 4.7 函数构建

#### 4.7.1 函数声明构建

`llvm::Value* astNode::IRBuildFunc()`为函数定义相关的IR生成，负责处理以下语法：

```yacas
funcDeclaration → typeSpecifier funcDec compoundStmt
funcDec → ID LPT paramList RPT | ID LPT RPT
```

定义一个函数的步骤可以简单分为六步：

- **获取返回值类型**：根据`typeSpecifier`，通过`getNodeType()`和`getLLVMType()`得到函数返回值的LLVM类型。
- **获取参数类型**：调用`getParamList()`获得参数，由此来获得参数类型。（`getParamList()`实现细节与`getVarList()`如出一辙）
- **根据以上两者构建函数类型**：调用`llvm::FunctionType::get(retType, funcArgType, /*isVarArg*/ false)`返回`llvm::FunctionType*`类型的函数类型`funcType`。
- **根据函数类型声明函数**：调用`llvm::Function::Create(funcType, llvm::GlobalValue::ExternalLinkage, string name, Module)`
- **存储函数参数**（获取参数的引用）：通过`func->arg_begin()`方法获取函数参数的迭代器，再根据`paramList`对函数参数重命名。
- **定义函数体**：通过`llvm::BasicBlock::Create(theContext, "entry", func)`创建函数代码块后调用`IRBuildCompoundStmt()`进行函数体IR生成。

#### 4.7.2 函数体构建

`llvm::Value* astNode::IRBuildCompoundStmt()`和`llvm::Value* astNode::IRBuildStmt()`控制函数体相关的IR生成，负责处理以下语法：

```yacas
compoundStmt → LCB localDec stmtList RCB
localDec → varDeclaration localDec | %empty
stmtList → stmt stmtList | %empty
stmt → expStmt | compoundStmt | selecStmt | iterStmt | retStmt
```

函数体包括局部变量声明语句和其他操作语句，局部变量声明调用`IRBuildVar(false)`即可。`stmt`包括了四种语句：表达式、选择分支语句、循环语句、返回（跳转）语句，按照`nodeValue`分别调用对应的IR生成函数。

### 4.8 表达式构建

`llvm::Value* astNode::IRBuildExp()`构建表达式，负责处理以下语法：

```yacas
exp → exp dbOper exp | sgOper exp | LPT exp RPT | ID | ID Array | ID funcCall | sgFactor
```

#### 4.8.1 二元运算表达式

##### 4.8.1.1 赋值语句

赋值语句需要考虑两点：

- 左值为一个符号表中已经存在的变量，需要获得其内存地址（即在符号表中查找到它），右值为表达式。
- 赋值的过程中若左值右值类型不一样，需要做隐式的类型转换，把右值类型转换为左值的类型。

为了获得左值的内存地址，调用函数`llvm::Value* astNode::IRBuildID()`。该函数先后查找局部变量表和全局变量表，查找到之后若变量非数组或者指针，则直接返回查找到的`Value*`；若为数组，则需要根据其下标访问对应元素。

类型转换部分详见 **4.5.2 类型转换**

##### 4.8.1.2 比较语句

比较运算两边都为表达式，无需考虑变量地址。若两边类型不同，需要将位数较少的类型转换为位数较多的类型。（不能为布尔类型）

类型转换后根据比较符调用对应的API即可。

##### 4.8.1.3 逻辑运算

逻辑运算两边同样都为表达式，且必须为布尔类型。

判断类型为布尔类型后根据逻辑运算符调用对应API即可。

##### 4.8.1.4 四则运算

四则运算两边同样为表达式，且不能为布尔类型。同样若两边类型不同，需要将位数较少的类型转换为位数较多的类型。

#### 4.8.2 一元运算表达式

支持的一元运算有取负（针对非布尔值）、取反（针对布尔值）和表达式前面加正号（值等于原表达式），判断类型后创建对应的运算即可。

#### 4.8.3 函数调用语句

将函数分为输入输出和其他函数。其中输入输出函数在`codeGen`类中特别定义。

其他函数分为有参函数的调用和无参函数的调用，根据API `llvm::IRBuilderBase::CreateCall()`填参数创建函数调用即可。

#### 4.8.4 单变量表达式与数组表达式

数组表达式和指针调用`IRBuildID()`获取内存地址后使用load指令来加载内存地址对应的内容即可。

一维数组的关键API如下：

```c++
Builder.CreateGEP(llvm::Value* arrayPtr, llvm::ArrayRef<llvm::Value*> indexList, name);
```

其直接作用就是提取被访问对象的内存地址，`CreateGEP()`需要准备两个索引值，内存地址偏移量的起始为数组指针变量（`arrayPtr`）所指向位置，第一个索引值为0，这个偏移量令下一个索引值的起始位置从 `arrayPtr + 0` 开始；而第二个索引值就是真正需要访问的数组下标值。

单变量表达式调用`IRBuildID()`获取内存地址即可。

#### 4.8.5 常量表达式

常量只是一个字面量或者字面值，不能直接用来操作，必须依附于其对应的类型的变量或者指针。

有五种类型常量：`INT`  `FLOAT`  `CHAR ` `BOOL` `STR`

##### 4.8.5.1 整数/浮点数类型常量

对于整数和浮点数，可以直接用`std::stoi`和`std::stof`将`nodeName`由字符串转变成对应类型的数据。

整数：

```c++
llvm::ConstantInt::get(theBuilder.getInt32Ty(), num);
```

浮点数：

```c++
llvm::ConstantFP::get(theBuilder.getFloatTy(), llvm::APFloat(num));
```

LLVM IR 中, 浮点数常量使用 `ConstantFP` 类进行表示, 它在内部将数值存储在 `APFloat` 中 (`APFloat` 有能力存储任意精度的浮点数常量)。

##### 4.8.5.2 字符类型常量

对于字符类型，需要将一般字符与转义字符进行区分。以转义字符`'\n'`为例，其特点就是第二个字符为转义符`\`，因此可根据每个字符的第二个字符来判断其是否为转义字符。

部分代码如下：

```c++
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
```

##### 4.8.5.3 字符串类型常量

llvm ir中的字符串是以全局常量`GlobalVariable`的形式存放的。

对于字符串常量，首先需要消除两边的`""`，这里使用`substr`实现。

字符串常量用`ConstantDataArray`类的`getString`方法来定义。`ConstantDataArray`是一个常量数组（即里面存放的元素为常量），元素类型可以是1/2/4/8-byte的整型常量或float/double常量。字符串是由字符（char）数组构成，字符（char）在ir中对应的类型是`i8`，所以可以以此来构建字符串常量。

```c
llvm::Constant* strConst = llvm::ConstantDataArray::getString(theContext, str, true);
```

构建完成后创建全局常量存放字符串：

```c++
 auto gStrPtr  = new llvm::GlobalVariable(/*Module=*/*generator->gModule,
                                                    /*Type=*/strConst->getType(),
                                                    /*isConstant=*/true,
                                                    /*Linkage=*/llvm::GlobalValue::PrivateLinkage,
                                                    /*Initializer=*/strConst,
                                                    /*Name=*/"strConstTmp");
            llvm::SmallVector<llvm::Value*, 2> indexVector;
            llvm::Value* const0 = llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(theContext), 0);
            indexVector.push_back(const0);
            indexVector.push_back(const0);
            llvm::ArrayRef<llvm::Value*> indexList(indexVector);
            llvm::Value* strPtr = theBuilder.CreateGEP(gStrPtr, indexList, "arrayPtrTmp");
```

### 4.9 条件分支构建

`llvm::Value* astNode::IRBuildSelec()`用于处理条件分支语句，负责语法为：

```yacas
selecStmt → IF LPT exp RPT stmt %prec LOWER_THAN_ELSE | IF LPT exp RPT stmt ELSE stmt
```

基本的条件分支结构为：

```c++
if xxx then
	then-logic
else
	else-logic
```

#### 4.9.1 条件判断处理

通过构建条件表达式获得其真值，之后再根据`condV`选择跳转到对应的块中。

```c++
llvm::Value* condV = this->childPtr[2]->IRBuildExp();
condV              = theBuilder.CreateICmpEQ(condV, llvm::ConstantInt::get(theBuilder.getInt1Ty(), 1), "ifCond");
```

#### 4.9.2 分支主体构建

设置三个基本块和一个条件选择指令，基本块分别为条件符合时的then块、条件不符合时的else块、条件分支结束后的merge块：

```c++
llvm::BasicBlock* thenBB      = llvm::BasicBlock::Create(theContext, "then", theFunction);
llvm::BasicBlock* elseBB      = llvm::BasicBlock::Create(theContext, "else", theFunction);
llvm::BasicBlock* mergeBB     = llvm::BasicBlock::Create(theContext, "ifcond", theFunction);

llvm::BranchInst* select      = theBuilder.CreateCondBr(condV, thenBB, elseBB);   // 插入条件分支语句的指令
```

若代码中不存在else，则直接跳转到merge块。

根据条件分支的逻辑可以写出代码：

```c++
//then:
    //    then-logic = ThenV
    //    goto MergeBB
theBuilder.SetInsertPoint(thenBB);
thenV = this->childPtr[4]->IRBuildStmt(); // 构建then的内容
theBuilder.CreateBr(mergeBB);           	// 插入跳转到Merge分支的指令
thenBB = theBuilder.GetInsertBlock();   	// 获取Then语句的出口

//else:
    //    else-logic = ElseV
    //    goto MergeBB
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
```

### 4.10 条件循环构建

`llvm::Value* astNode::IRBuildIter()`用于处理条件循环语句，负责的语法为：

```yacas
iterStmt → WHILE LPT exp RPT stmt
```

基本的循环结构为：

```c++
while xxx
  loop-block
```

#### 4.10.1 循环主体构建

设置三个基本块，分别为判断循环条件的cond块、循环主体body块和循环结束后的end块：

```c++
llvm::BasicBlock* condBB      = llvm::BasicBlock::Create(theContext, "cond", theFunction);
llvm::BasicBlock* bodyBB      = llvm::BasicBlock::Create(theContext, "body", theFunction);
llvm::BasicBlock* endBB       = llvm::BasicBlock::Create(theContext, "end", theFunction);
```

与条件分支只需要判断一次条件不同的是，条件循环需要在每次循环后都判断一次条件是否满足，以确定是否继续循环。因此将循环条件判断独立为`condBB`块。

根据循环逻辑可写出代码：

```c++
		theBuilder.CreateBr(condBB);   // 跳转到条件分支

    // condBB基本块
    theBuilder.SetInsertPoint(condBB);
    llvm::Value* condV       = this->childPtr[2]->IRBuildExp();
    condV                    = theBuilder.CreateICmpEQ(condV, llvm::ConstantInt::get(theBuilder.getInt1Ty(), 1), "whileCond");
    llvm::BranchInst* select = theBuilder.CreateCondBr(condV, bodyBB, endBB);   // 插入分支语句的指令
    condBB                   = theBuilder.GetInsertBlock();                     // 获取条件语句的出口

    // bodyBB基本块
    theBuilder.SetInsertPoint(bodyBB);
    this->childPtr[4]->IRBuildStmt();
    theBuilder.CreateBr(condBB);   // 跳转到condBB

    // endBB基本块
    theBuilder.SetInsertPoint(endBB);
```

#### 4.10.2 break实现

`break`语句是控制条件循环的重要语句，其本质是一个跳转指令，因此放在`IRBuildRet()`中构建。

当满足`break`的条件时，循环从当前的`condBB`块跳转到`endBB`块。为了实现嵌套循环的`break`，需要保存每一层循环的`endBB`块，因此在`codeGen`类中设置一个`blockStack`栈来保存每次循环的`endBB`。每一层循环开始时push，结束时pop。

`break`只需要跳转到栈顶块即可：

```c++
theBuilder.CreateBr(generator->blockStack.top())
```

### 4.11 Return语句构建

`llvm::Value* astNode::IRBuildRet()`为return语句实现，负责语法为：

```yacas
retStmt → RETURN exp SEMICOLON | RETURN SEMICOLON | BREAK SEMICOLON
```

其中`break`语句已在 **4.10.2 break实现** 中介绍，剩余二者只需要根据返回值选择`theBuilder.CreateRet(llvm::Value*)`或`theBuilder.CreateRetVoid()`进行构建即可。

### 4.12 输出函数构建

本项目设置了三种输出

- `print()`：输出括号内的内容，包括常量和变量
- `println()`：输出括号内内容后换行
- `printf()`：格式化输出

输出函数声明如下：

```c++
    std::vector<llvm::Type*> printfArgs;
    printfArgs.push_back(theBuilder.getInt8Ty()->getPointerTo());
    llvm::ArrayRef<llvm::Type*> argsRef(printfArgs);
    llvm::FunctionType*         printfType = llvm::FunctionType::get(theBuilder.getInt32Ty(), argsRef, true);
    llvm::Function*             printfFunc = llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, llvm::Twine("printf"), gModule);
    printfFunc->setCallingConv(llvm::CallingConv::C);
    return printfFunc;
```

具体实现参考 [SPL-Compiler](https://github.com/LittleJohnKhan/SPL-Complier)

## 5. 代码生成

### 5.1 codeGen类

本模块主要控制LLVM IR生成并定义IR生成时所需的部分数据、结构等。相关文件为`codeGen.h`和`codeGen.cpp`。

`codeGen`类

```c++
class codeGen
{
public:
    // 用于管理函数和全局变量，类似于类c++的编译单元(单个cpp文件)
    llvm::Module* gModule;
    // 块栈，用于多重循环break
    std::stack<llvm::BasicBlock*> blockStack;
    llvm::Function*               printf;
    llvm::Function* printGen();
    void codeGenerator(astNode* root);
    codeGen();
    ~codeGen();
};
```

### 5.2 Makefile

编译流程为首先flex+bison联合编译，扫描源代码后生成AST；然后调用LLVM IR C++API生成中间表示文件`.ll`，用`llvm-as`工具通过中间表示文件（`.ll` 文件）得到字节码文件（`.bc`文件）；最后用clang将字节码文件直接编译为可执行文件。

```makefile
NAME 		= cmm
CC			= clang++ -std=c++17
OBJS 		= gramGen.o tokenGen.o main.o codeGen.o ast.o
LLVM_DIR 	= /opt/homebrew/opt/llvm
LLVM_AS 	= ${LLVM_DIR}/bin/llvm-as
LLVM_LIB 	= -L ${LLVM_DIR}/lib
LLVM_INCLU	= -I ${LLVM_DIR}/include

.PHONY: clean

build:
	flex -o tokenGen.cpp tokenGen.l
	bison -o gramGen.cpp gramGen.y
	$(CC) $(LLVM_INCLU) $(LLVM_LIB) -lLLVM ./*.cpp -o diana

test:
	cat ../test/test.cmm | ./diana > ../test/test.ll
	$(LLVM_AS) ../test/test.ll -o ../test/test.bc	
	$(CC) ../test/test.bc -o ../test/test
	../test/test
	../test/test > ../test/result.txt
clean:
	$(RM) -f $(OBJS) gramGen.cpp gramGen.hpp diana tokenGen.cpp
	$(RM) -f ../test/*.ll ../test/*.bc ../test/test
	$(RM) -f ../test/*.txt

cleantest:
	$(RM) -f ../test/*.ll ../test/*.bc ../test/test
```

## 6. 前端设计

本项目为编译器设计了一个简单的web IDE，可以实现代码编写、文件打开与保存、代码编译运行、显示结果等功能。

### 6.1 整体布局

顶部tabbar实现的功能从左到右为：google搜索、打开/关闭代码区、打开本地文件、保存文件、编译运行代码、显示结果、github链接、时间日期。

底部有弹出式工具栏。

当时间为6:00～18:00时背景如下

<img src="/Users/skyyyy/Documents/2022_SPRSUM/compiler/myCompiler/docs/report.assets/截屏2022-05-29 11.54.01.png" alt="截屏2022-05-29 11.54.01" style="zoom:50%;" />

当时间为6:00～18:00时背景如下

<img src="/Users/skyyyy/Documents/2022_SPRSUM/compiler/myCompiler/docs/report.assets/截屏2022-05-29 12.01.21.png" alt="截屏2022-05-29 12.01.21" style="zoom:50%;" />

通过以下代码得到时间并实现背景切换：

```javascript
//创建数组存放背景url
var bgs = new Array('url("images/background/sky.png")', 'url("images/background/night.png")');
//插入背景函数
function insertBg() {
    var now = new Date();
    var hour = now.getHours();
    if (hour >= 6 && hour < 18) {
        document.getElementById('bgid').style.backgroundImage = bgs[0];
    } else if ((hour >= 18 && hour < 24) || (hour >= 0 && hour < 6)) {
        document.getElementById('bgid').style.backgroundImage = bgs[1];
    }
}
```

### 6.2 TabBar功能

#### 6.2.1 搜索栏

```html
                <div class="searchbox">
                    <form action="http://www.google.com/search" method="get" target="_blank">
                        <input type="search" name="q" id="seaid" placeholder="Heil, diana!" autofocus="autofocus" autocomplete="off">
                        <input type="submit" id="subid" value="">
                    </form>
                </div>
```

#### 6.2.2 代码区

点击Code按钮，可实现代码区和结果区的弹出与收回。

<img src="/Users/skyyyy/Documents/2022_SPRSUM/compiler/myCompiler/docs/report.assets/截屏2022-05-29 12.05.02.png" alt="截屏2022-05-29 12.05.02" style="zoom:50%;" />

<img src="/Users/skyyyy/Documents/2022_SPRSUM/compiler/myCompiler/docs/report.assets/截屏2022-05-29 13.40.04.png" alt="截屏2022-05-29 13.40.04" style="zoom:50%;" />

采用文本框+codemirror组件实现clike语言的代码高亮、代码折叠、智能锁进、括号匹配等。

<img src="/Users/skyyyy/Documents/2022_SPRSUM/compiler/myCompiler/docs/report.assets/截屏2022-05-29 12.10.12.png" alt="截屏2022-05-29 12.10.12" style="zoom:50%;" />

相关代码：

```javascript
// codeblock codemirror构建代码框
var editor = CodeMirror.fromTextArea(document.getElementById("txt"), {
    mode: "text/x-java", //实现Java代码高亮
    mode: "text/x-c++src",
    lineNumbers: true,//是否显示每一行的行数
    indentUnit: 4, //缩进单位为4
    matchBrackets: true, //括号匹配
    styleActiveLine: true, //当前行背景高亮
    readOnly: false,//只读
    lineWrapping: true,	//代码折叠
    foldGutter: true,
    smartIndent: true,//智能缩进
    gutters: ["CodeMirror-linenumbers", "CodeMirror-foldgutter"],
    lineWrapping: true, //自动换行
    theme: 'ayu-mirage', //编辑器主题
});
//var height = 650;
editor.setSize('100%', '100%');

// resultblock codemirror构建代码框
var resultEdi = CodeMirror.fromTextArea(document.getElementById("result"), {
    mode: "text/x-java", //实现Java代码高亮
    mode: "text/x-c++src",
    lineNumbers: true,//是否显示每一行的行数
    indentUnit: 4, //缩进单位为4
    matchBrackets: true, //括号匹配
    styleActiveLine: true, //当前行背景高亮
    readOnly: true,//只读
    lineWrapping: true,	//代码折叠
    foldGutter: true,
    smartIndent: true,//智能缩进
    gutters: ["CodeMirror-linenumbers", "CodeMirror-foldgutter"],
    lineWrapping: true, //自动换行
    theme: 'ayu-mirage', //编辑器主题
});
//var height = 650;
resultEdi.setSize('100%', '100%');
```

#### 6.2.3 文件打开

点击Open按钮可以打开本地文件并将文件内容写入代码区。

<img src="/Users/skyyyy/Documents/2022_SPRSUM/compiler/myCompiler/docs/report.assets/截屏2022-05-29 12.17.04.png" alt="截屏2022-05-29 12.17.04" style="zoom:50%;" />

```javascript
    $('#fileInput').trigger('click');
    document.getElementById('fileInput').addEventListener('change', function selectedFileChanged() {
        if (this.files.length == 0) {
            console.log('Choose file nia~');
            return;
        }
        const reader = new FileReader();
        reader.onload = function fileReadCompleted() {
            //当读取完成时，内容只在`reader.result`中
            console.log(reader.result);
            var content = reader.result;
            editor.setValue(content);
        };
        reader.readAsText(this.files[0]);
    });
```

#### 6.2.4 文件保存

点击保存按钮将代码区内容保存为.cmm文件。

<img src="/Users/skyyyy/Documents/2022_SPRSUM/compiler/myCompiler/docs/report.assets/截屏2022-05-29 12.19.34.png" alt="截屏2022-05-29 12.19.34" style="zoom:50%;" />

#### 6.2.5 编译运行

原计划使用xterm写一个小型web终端，或者利用flask运行后端代码后一键显示结果，但由于时间原因未能实现。

#### 6.2.6 显示结果

在`makefile`中将结果输出到`result.txt`中，选择该文件输出到结果区。

<img src="/Users/skyyyy/Documents/2022_SPRSUM/compiler/myCompiler/docs/report.assets/截屏2022-05-29 13.47.10.png" alt="截屏2022-05-29 13.47.10" style="zoom:50%;" />

### 6.3 底部菜单栏

分别为 bilibili、学在浙大、github、gitee链接。

<img src="/Users/skyyyy/Documents/2022_SPRSUM/compiler/myCompiler/docs/report.assets/截屏2022-05-29 13.47.40.png" alt="截屏2022-05-29 13.47.40" style="zoom:50%;" />

## 7. 测试

测试样例为输出斐波那契数列前20个数据：

```c++
/*
 * @Author: Skyyyy
 * @Date: 2022-05-18 20:55:09
 * @Description: Heil Diana
 */
int x[100];
int fib(int i)
{
    int a;
    int b;
    if (i == 0)
    {
        return 1;
    }
    if (i == 1)
    {
        return 1;
    }
    a = fib(i - 1);
    b = fib(i - 2);
    return a + b;
}
int main()
{
    int i;
    int num;
    i   = 0;
    num = 50;
    // scan(num);
    while (i < num)
    {
        x[i] = fib(i);
        // println(x[i]);
        printf("%d", x[i]);
        println("");
        i = i + 1;
        if (i == 20)
        {
            break;
        }
    }
    return 0;
}
```

其测试范围包括了局部/全局变量声明、局部/全局变量赋值、while循环及break、if语句、return语句、表达式、函数调用、递归。

<img src="/Users/skyyyy/Documents/2022_SPRSUM/compiler/myCompiler/docs/report.assets/截屏2022-05-29 13.51.57.png" alt="截屏2022-05-29 13.51.57" style="zoom:50%;" />

## 附：参考资料

[Flex&Bison入门](https://blog.csdn.net/weixin_44007632/article/details/108666375)

[C++可变参数函数介绍](https://songlee24.github.io/2014/07/22/cpp-changeable-parameter/)

[LLVM官网](https://llvm.org/doxygen/index.html)

[LLVM教程](https://releases.llvm.org/5.0.0/docs/tutorial/index.html)

[LLVM每日谈](https://www.zhihu.com/column/llvm-clang)

[IRBuilder生成LLVM IR](http://blog.silence.pink/2021/03/14/create-llvmIR-by-IRBuilder/)

[LLVM IR C++API使用参考](https://blog.csdn.net/qq_42570601/category_10200372.html)

[How to create GlobalVar - Stackoverflow](https://stackoverflow.com/questions/7787308/how-can-i-declare-a-global-variable-in-llvm)

[LLVM IR 字符串、全局变量、全局常量及数组生成](https://blog.csdn.net/qq_42570601/article/details/108007986)

[LLVM IR 局部变量生成](https://zhuanlan.zhihu.com/p/433043819)

[LLVM IR 局部变量生成2](https://blog.csdn.net/mrpre/article/details/106902346)

[LLVM IR 函数定义](https://blog.csdn.net/qq_42570601/article/details/108059403)

[llvm::Instruction::CastOps 操作](https://llvm.org/docs/LangRef.html#constant-expressions)

[创建、访问数组](https://www.twblogs.net/a/5e4fb5efbd9eee101df85956)

[LLVM IR if生成](https://tin.js.org/2020/07/09/llvm-1/)

[if参考2](https://blog.csdn.net/mrpre/article/details/106900611)

[if和while生成](https://blog.csdn.net/qq_42570601/article/details/107771289)

[SPL-Compiler](https://github.com/LittleJohnKhan/SPL-Complier)

[llvm编译指令](https://blog.csdn.net/pc153262603/article/details/89553688)
