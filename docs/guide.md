## myCompiler
a simple Cminus compiler based on Cpp

macOS

flex 2.6.4

bison (GNU Bison) 3.8.2

llvm 13.0.1

### 1. 文件组织结构

```apl
myCompiler
├── src
│   ├── cmm.l
│   ├── cmm.y
│   ├── ast.h
│   ├── ast.cpp
│   ├── codeGen.h 
│   ├── codeGen.cpp 
│   ├── def.h
│   ├── main.cpp  
│   └── Makefile          
│   
├── test
│ 
└── js
    ├── d3.js
    ├──
    └──
```

### 2. 相关参考 & 代码规范

[Lex&Yacc 联合原理](https://zhuanlan.zhihu.com/p/143867739)

[yacc中避免if和if else冲突](https://stackoverflow.com/questions/1737460/how-to-find-shift-reduce-conflict-in-this-yacc-file)

[可变参数函数](https://songlee24.github.io/2014/07/22/cpp-changeable-parameter/)

[LLVM官方文档](https://llvm.org/doxygen/namespacellvm.html)

[LLVM教程](https://llvm-tutorial-cn.readthedocs.io/en/latest/chapter-1.html)

[LLVM IR相关语法与接口 ⚠️重要参考](https://blog.csdn.net/qq_42570601/category_10200372.html)

[LLVM编译器架构](https://blog.csdn.net/xfxyy_sxfancy/category_9264747.html?spm=1001.2014.3001.5482)

[LLVM例程](https://cloud.tencent.com/developer/article/1879336?from=article.detail.1352547)

[LLVM中文](https://llvm.liuxfe.com/tutorial/langimpl/)

[IRBuilderBase API](https://llvm.org/doxygen/classllvm_1_1IRBuilderBase.html#a06d64f226fabd94d694d6abd82e3c7b3)

[LLVM IR 分支codeGen ⚠️有用](https://tin.js.org/2020/07/09/llvm-1/)

[LLVMIR exp生成](https://hlli.xyz/index.php/archives/15/)

[LLVM IR 局部变量生成](https://blog.csdn.net/mrpre/article/details/106902346)

[GetElementPtr GEP](http://llvm.org/docs/GetElementPtr.html#introduction)

[创建 使用数组 GEP](https://www.twblogs.net/a/5e4fb5efbd9eee101df85956)

[有用的llvm api](https://llvm.liuxfe.com/docs/ProgrammersManual/important-and-useful-llvm-apis)

[llvm::Instruction::CastOps 操作](https://llvm.org/docs/LangRef.html#constant-expressions)

[LLVM 调用print输出int float](https://www.cxyzjd.com/article/oWuYeFeiYing/44966047)

[一个LLVM相关实验](https://clarazhang.gitbooks.io/compiler-f2017/content/)

[printf 参考](https://blog.csdn.net/xfxyy_sxfancy/article/details/49687653)

[使用IRBuilder生成LLVM IR ](http://blog.silence.pink/2021/03/14/create-llvmIR-by-IRBuilder/)

[王强高徒 完美熟肉](https://github.com/LittleJohnKhan/SPL-Complier)

[d3.js 绘制树状图](https://juejin.cn/post/6844903998965678093)

**规范：**

变量定义采用小驼峰

不使用`using namespace std`，使用std库时手动添加`std::`

### 3. 词法分析模块

词法分析负责ast的leaf节点构建（将每个终结符作为leaf节点），语法分析负责剩余节点构建

### 4. 语法分析模块

#### 4.1 cmm.y

**定义的BNF语法**

全大写表示终结符，采用自顶向下分析

左递归化为右递归

```yacas
Program → ExtDefList
ExtDefList → ExtDef ExtDefList | %empty
ExtDef → Specifier ExtDecList SEMI | Specifier FunDec Compst
ExtDecList → VarDec | VarDec COMMA ExtDecList
Specifier → TYPE
VarDec → ID | ID LB INT RB | ID LB RB
FunDec → ID LP VarList RP | ID LP RP
VarList → ParamDec COMMA VarList | ParamDec
ParamDec → Specifier VarDec
CompSt → LC DefList StmtList RC
StmtList → Stmt StmtList | %empty
Stmt → Exp SEMI | CompSt | RETURN Exp SEMI | RETURN SEMI | BREAK SEMI | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE | IF LP Exp RP Stmt ELSE Stmt | WHILE LP Exp RP Stmt
DefList → Def DefList | %empty
Def → Specifier DecList SEMI
DecList → VarDec | VarDec COMMA DecList
Exp → Exp ASSIGNOP Exp | Exp AND Exp | Exp OR Exp | Exp RELOP Exp | Exp PLUS Exp | Exp MINUS Exp | Exp STAR Exp | Exp DIV Exp | LP Exp RP | MINUS Exp | NOT Exp | ID LP Args RP | ID LP RP | ID LB Exp RB | ID LB RB | ID | INT | FLOAT | BOOL | CHAR | STRING
Args → Exp COMMA Args | Exp
```

```yacas
program → decList
decList → dec decList | %empty
dec → varDeclaration | funcDeclaration 🍺

varDeclaration → typeSpecifier varDecList SEMICOLON
typeSpecifier → TYPE
varDecList → varDef | varDef COMMA varDecList
varDef → ID | ID LSB INT RSB

funcDeclaration → typeSpecifier funcDec compoundStmt 🍺
funcDec → ID LPT paramList RPT | ID LPT RPT 🍺
paramList → paramDec COMMA paramList | paramDec 🍺
paramDec → typeSpecifier paramDef 🍺
paramDef → ID | ID LSB RSB 🍺
compoundStmt → LCB localDec stmtList RCB 🍺
localDec → varDeclaration localDec | %empty 🍺
stmtList → stmt stmtList | %empty 🍺
stmt → expStmt | compoundStmt | selecStmt | iterStmt | retStmt 🍺
selecStmt → IF LPT exp RPT stmt %prec LOWER_THAN_ELSE | IF LPT exp RPT stmt ELSE stmt 🍺
iterStmt → WHILE LPT exp RPT stmt 🍺
retStmt → RETURN exp SEMICOLON | RETURN SEMICOLON | BREAK SEMICOLON 🍺
expStmt → exp SEMICOLON | SEMICOLON
exp → exp dbOper exp | sgOper exp | LPT exp RPT | ID | ID Array | ID funcCall | sgFactor
dbOper → PLUS | MINUS | MULTI | DIV | RELOP | ASSIGN | AND | OR
sgOper → MINUS | NOT | PLUS
Array → LSB exp RSB | LSB RSB
funcCall → LPT argList RPT | LPT RPT
argList → exp COMMA argList | exp 🍺
sgFactor → INT | FLOAT | CHAR | BOOL | STR
```



```yacas
======================================================= IRBuilder()
program → decList
decList → dec decList | %empty
dec → varDeclaration | funcDeclaration 🍺
=======================================================

======================================================= IRBuildVar()
varDeclaration → typeSpecifier varDecList SEMICOLON

======================================================= getNodeType()
typeSpecifier → TYPE
=======================================================

======================================================= getVarList()
varDecList → varDef | varDef COMMA varDecList
varDef → ID | ID LSB INT RSB
=======================================================
======================================================= IRBuildFunc()
funcDeclaration → typeSpecifier funcDec compoundStmt 🍺
funcDec → ID LPT paramList RPT | ID LPT RPT 🍺
=======================================================
======================================================= getParamList()
paramList → paramDec COMMA paramList | paramDec 🍺
paramDec → typeSpecifier paramDef 🍺
paramDef → ID | ID LSB RSB 🍺
=======================================================
======================================================= IRBuildCompoundStmt()
compoundStmt → LCB content RCB 🍺
content → localDec stmtList | %empty 🍺
localDec → varDeclaration localDec | %empty 🍺
stmtList → stmt stmtList | %empty 🍺
=======================================================
======================================================= IRBuildStmt()
stmt → expStmt | compoundStmt | selecStmt | iterStmt | retStmt 🍺
expStmt → exp SEMICOLON | SEMICOLON
=======================================================

selecStmt → IF LPT exp RPT stmt %prec LOWER_THAN_ELSE | IF LPT exp RPT stmt ELSE stmt 🍺
iterStmt → WHILE LPT exp RPT stmt 🍺
retStmt → RETURN exp SEMICOLON | RETURN SEMICOLON | BREAK SEMICOLON 🍺

======================================================= IRBuildExp()
exp → exp dbOper exp | sgOper exp | LPT exp RPT | ID | ID Array | ID funcCall | sgFactor
dbOper → PLUS | MINUS | MULTI | DIV | RELOP | ASSIGN | AND | OR
sgOper → MINUS | NOT | PLUS
Array → LSB exp RSB | LSB RSB
funcCall → LPT argList RPT | LPT RPT
======================================================= getArgList()
argList → exp COMMA argList | exp 🍺
=======================================================
sgFactor → INT | FLOAT | CHAR | BOOL | STR
```

#### 4.2 AST

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
    //相关值的method以及llvm提供的cpp接口
};
```

关于可变参数，C++11提供了两种主要的方法：如果所有的实参类型相同，可以传递一个名为**initializer_list**的标准库类型；如果实参的类型不同，则编写**可变参数模板**。

### 5. 语义分析模块 IR

 LLVM

```yacas
Source Code -> | Frontend | Optimizer | Backend | -> Machine Code
```

LLVM的一大特色就是有着独立的、完善的、严格约束的中间代码表示。这种中间代码，就是LLVM的字节码前端生成这种中间代码，后端自动进行各类优化分析。

#### 5.1 Runtime

```cpp
// 记录了LLVM的核心数据结构，比如类型和常量表，不过不太需要关心它的内部
llvm::LLVMContext theContext;
// 用于创建LLVM指令
llvm::IRBuilder<> Builder(theContext);
// 用于管理函数和全局变量，类似于类c++的编译单元(单个cpp文件)
llvm::Module* gModule = new llvm::Module("myCMM", theContext);
```

#### 5.2 符号表

theContext

#### 5.3 类型操作 🍺

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

#### 5.4 类型转换（隐式）

[IR类型转换](https://zhuanlan.zhihu.com/p/163063995)

`CreateCast`

#### 5.5 变量处理

类型 变量名

`varList`保存变量名和类型（var or array，arraysize=0则为ptr）

判断是否为全局变量，插入符号表。

**⚠️如何判断全局变量与局部变量？---------------------待写**

🍺 暂且用一个flag `isGobalVar` 表示，由上层调用者传入 (✖️)

🍺 传入当前函数指针 `llvm::Function* func`，全局变量该指针为`nullptr`，通过判断该指针来区分

##### 5.5.1 全局变量

[How to create GlobalVar - Stackoverflow](https://stackoverflow.com/questions/7787308/how-can-i-declare-a-global-variable-in-llvm)

[重要参考](https://blog.csdn.net/qq_42570601/article/details/108007986)

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

// 全局变量数组
//定义一个int类型的全局数组常量

	llvm::ArrayType* array_type = llvm::ArrayType::get(llvm::Type::getInt32Ty(context), 4);
	llvm::GlobalVariable* array_global = new llvm::GlobalVariable(/*Module=*/*module,
		/*Type=*/array_type,
		/*isConstant=*/true,
		/*Linkage=*/llvm::GlobalValue::PrivateLinkage,
		/*Initializer=*/0, // has initializer, specified below
		/*Name=*/"array_global");
	array_global->setAlignment(16);
	std::vector<llvm::Constant*> const_array_elems;
	const_array_elems.push_back(con_1);
	const_array_elems.push_back(con_2);
	const_array_elems.push_back(con_3);
	const_array_elems.push_back(con_4);
	llvm::Constant* const_array = llvm::ConstantArray::get(array_type, const_array_elems);//数组常量
	array_global->setInitializer(const_array);//将数组常量初始化给全局常量
```

**⚠️另外需要判断全局变量是否重复定义------------------待写**

` llvm::getGlobalVariable(name) `在全局变量表中查找全局变量

```c++
if (gModule->getGlobalVariable(it->second, true) != NULL)
{
	throw("GlobalVar Name Duplicated.\n");
}
```

##### 5.5.2 局部变量

**⚠️局部变量部分如何写---------------------------待写**

插入对应函数的符号表

编写一个函数 CreateEntryBlockAlloca，简化后续工作，其功能是往函数的 EntryBlock 的最开始的地方添加分配内存指令：

```c++
llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* func,
                                         const std::string&amp; var_name) {
  llvm::IRBuilder<> ir_builder(&amp;(func->getEntryBlock()),
                               func->getEntryBlock().begin());
  return ir_builder.CreateAlloca(llvm::Type::getDoubleTy(g_llvm_context), 0,
                                 var_name.c_str());
}
```

局部变量的查找：

```cpp
       // 查找局部变量
   llvm::Value*     var = func->getValueSymbolTable()->lookup(std::string varName);
        if (var == nullptr)
        {
            throw("Var Undeclared\n");
        }
```



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

[分支&循环参考](https://blog.csdn.net/qq_42570601/article/details/107771289)

##### 5.7.1 if else

```cpp
llvm::IRBuilderBase::CreateCondBr(Cond, True, False);
```

[分支参考](https://tin.js.org/2020/07/09/llvm-1/)

```c++
// If语句
if 1 < 2 
   3
else f(1, 2, 3);

Value *CondV = Cond->codegen();
CondV = Builder.CreateFCmpONE(CondV, ConstantFP::get(TheContext, APFloat(0.0)), "ifcond");
Function *TheFunction = Builder.GetInsertBlock()->getParent();
BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", TheFunction); // 自动加到函数中
BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else");
BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont");

Builder.CreateCondBr(CondV, ThenBB, ElseBB); // 插入条件分支语句的指令

// Then语句处理
Builder.SetInsertPoint(ThenBB);
Value *ThenV = Then->codegen();
Builder.CreateBr(MergeBB); // 插入跳转到Merge分支的指令
ThenBB = Builder.GetInsertBlock(); // 获取Then语句的出口

// Else语句处理
TheFunction->getBasicBlockList().push_back(ElseBB); // 添加到函数中去
Builder.SetInsertPoint(ElseBB);
Value *ElseV = Else->codegen();
Builder.CreateBr(MergeBB); // 插入跳转到Merge分支的指令
ElseBB = Builder.GetInsertBlock(); // 获取Else语句的出口

// PHI指令的生成
TheFunction->getBasicBlockList().push_back(MergeBB);
Builder.SetInsertPoint(MergeBB);
PHINode *PN = Builder.CreatePHI(Type::getDoubleTy(TheContext), 2, "iftmp");
PN->addIncoming(ThenV, ThenBB);
PN->addIncoming(ElseV, ElseBB);
```

##### 5.7.2 while

```cpp
while(i <= n){
		sum = sum + i;
		i++;
	}

//创建循环使用到的三个代码块
	BasicBlock *while_count = BasicBlock::Create(context, "while_count", sum_fun);
	BasicBlock *while_body = BasicBlock::Create(context, "while_body", sum_fun);
	BasicBlock *while_end = BasicBlock::Create(context, "while_end", sum_fun);
	
	//给变量i和sum申请内存,并放入初值0
	Value* i_alloca = builder.CreateAlloca(Type::getInt32Ty(context));
	Value* sum_alloca = builder.CreateAlloca(Type::getInt32Ty(context));
	builder.CreateStore(con_0, i_alloca);
	builder.CreateStore(con_0, sum_alloca);
	builder.CreateBr(while_count);
	
	//while_count基本块
	builder.SetInsertPoint(while_count);
	Value* i_load = builder.CreateLoad(Type::getInt32Ty(context), i_alloca);
	Value *cmp_value = builder.CreateICmpSLE(i_load, arg_n);
	//根据cmp的值跳转，也就是if条件
	builder.CreateCondBr(cmp_value, while_body, while_end);
	
//while_body基本块
	builder.SetInsertPoint(while_body);
	//sum = sum + i;
	Value* sum_load = builder.CreateLoad(Type::getInt32Ty(context), sum_alloca);
	Value* temp_sum = builder.CreateAdd(sum_load, i_load);
	builder.CreateStore(temp_sum, sum_alloca);
	//i++;
	Value* temp_i = builder.CreateAdd(i_load, con_1);
	builder.CreateStore(temp_i, i_alloca);
	builder.CreateBr(while_count);
	
	//while_end基本块
	builder.SetInsertPoint(while_end);
```

```c++
for i = 0, i < 100, 1.00 in
  f(1, 2, i);

Value *StartVal = Start->codegen();
Function *TheFunction = Builder.GetInsertBlock()->getParent();
BasicBlock *PreheaderBB = Builder.GetInsertBlock();
BasicBlock *LoopBB = BasicBlock::Create(TheContext, "loop", TheFunction);
Builder.CreateBr(LoopBB); // 跳转到Loop分支

Builder.SetInsertPoint(LoopBB);
// 创建PHI节点
PHINode *Variable = Builder.CreatePHI(Type::getDoubleTy(TheContext), 2, VarName.c_str());
Variable->addIncoming(StartVal, PreheaderBB);
NamedValues[VarName] = Variable; // 将for定义的变量添加到作用域中
Body->codegen();
Value *StepVal = Step->codegen();
Value *NextVar = Builder.CreateFAdd(Variable, StepVal, "nextvar");
Value *EndCond = End->codegen();
EndCond = Builder.CreateFCmpONE(EndCond, ConstantFP::get(TheContext, APFloat(0.0)), "loopcond");
BasicBlock *LoopEndBB = Builder.GetInsertBlock(); // 为啥不可以直接使用LoopBB，而是还要获取一次呢？
BasicBlock *AfterBB = BasicBlock::Create(TheContext, "afterloop", TheFunction);
Builder.CreateCondBr(EndCond, LoopBB, AfterBB);

Builder.SetInsertPoint(AfterBB);
Variable->addIncoming(NextVar, LoopEndBB);
```

#### 5.8 表达式

##### 5.8.1 常量 sgFactor

###### 5.8.1.1 Float get

LLVM IR 中, numeric constants 使用 `ConstantFP` 类进行表示, 它在内部将数值 hold 在 `APFloat` 中 (`APFloat` 有能力 hold 任意精度的浮点数常量)。这段代码基本上创建并返回一个 `ConstantFP`。注意在LLVM IR 中 constants 是独特且共享的。因为这个理由, API 使用 “foo::get(…)” 写法而非 “new foo(..)” 或者 “foo::Create(..)”.

```c++
	982	Constant *ConstantFP::get(Type *Ty, const APFloat &V) {
  983   ConstantFP *C = get(Ty->getContext(), V);
  984   assert(C->getType() == Ty->getScalarType() &&
  985          "ConstantFP type doesn't match the type implied by its value!");
  986  
  987   // For vectors, broadcast the value.
  988   if (auto *VTy = dyn_cast<VectorType>(Ty))
  989     return ConstantVector::getSplat(VTy->getElementCount(), C);
  990  
  991   return C;
  992 }
```

###### 5.8.1.2 字符串

[参考](https://blog.csdn.net/qq_42570601/article/details/108007986)

字符串常量用ConstantDataArray类的getString方法来定义。ConstantDataArray是一个常量数组（即里面存放的元素时常量），元素类型可以是1/2/4/8-byte的整型常量或float/double常量。字符串是由字符（char）数组构成，字符（char）在ir中对应的类型是i8，所以可以以此来构建字符串常量。

```c++
//创建字符串常量
	llvm::Constant *strConst1 = llvm::ConstantDataArray::getString(context,
			"exception_name");
	llvm::Value *globalVar1 = new llvm::GlobalVariable(*module,
			strConst1->getType(), true, llvm::GlobalValue::PrivateLinkage,
			strConst1, "globalVar1");
	llvm::Constant *strConst2 = llvm::ConstantDataArray::getString(context,
			"uuid");
	llvm::Value *globalVar2 = new llvm::GlobalVariable(*module,
			strConst2->getType(), true, llvm::GlobalValue::PrivateLinkage,
			strConst2, "globalVar2");

llvm::SmallVector<llvm::Value*, 2> indexVector;
llvm::Value *const_0 = llvm::ConstantInt::get(
			llvm::IntegerType::getInt32Ty(context), 0);
indexVector.push_back(const_0);
	indexVector.push_back(const_0);
	llvm::Value *number_ptr_1 = builder.CreateGEP(alloca_Struct, indexVector);
```

一维数组

假设有数组 int a[10] = {0}，现需要访问下标为3的元素值（即a[3]），使用IRBuilder.CreateInBoundsGEP()方法，则代码如下：

```cpp
// argVarArr contains the array a[10]'s pointer
Value* arrVal = Builder.CreateAlignedLoad(argVarArr, 8);
// index value
Constant* three = llvm::ConstantInt::get(M.getContext(), llvm::APInt(32, 3, true));
// set the element's type: i32
Types* eleTy = llvm::Type::getInt32Ty(M.getContext());
// get the element a[3]'s pointer
// [ %arrayidx = getelementptr inbounds i32, i32* %a, i32 3 ]
Value* elePtr = Builder.CreateInBoundsGEP(eleTy, arrVal, three);
// get the element a[3]'s value
// [ %4 = load i32, i32* %arrayidx, align 4 ]
Value* eleVal = Builder.CreateAlignedLoad(elePtr, 4);
```

使用IRBuilder.CreateGEP()方法则稍微麻烦，需要准备两个索引值，内存地址偏移量的起始仍然是数组指针变量（arrVal）所指向位置，第一个索引值为0，这个偏移量令下一个索引值的起始位置以 arrVal + 0 开始.[链接](http://llvm.org/docs/GetElementPtr.html#introduction)

##### 5.8.2 函数调用

```c++
Value *CallExprAST::Codegen() {
  // Look up the name in the global module table.
  Function *CalleeF = TheModule->getFunction(Callee);
  if (CalleeF == 0)
    return ErrorV("Unknown function referenced");

  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size())
    return ErrorV("Incorrect # arguments passed");

  std::vector<Value*> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->Codegen());
    if (ArgsV.back() == 0) return 0;
  }
  return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}
```



#### 5.9 返回&跳转语句

##### 5.9.1 return exp;

```c++
Value* ret = builder.CreateFPToSI(ret_pow, Type::getInt32Ty(context));
return builder.CreateRet(ret);
```

##### 5.9.2 return;

```c++
return Builder.CreateRetVoid();
```

##### 5.9.3 break;

**⚠️-----------------------待写**

🍺 暂且让break返回一个nullptr，在whileblock中判断循环体到返回值

#### 5.10 输入输出函数

**⚠️ 能不能添加格式化输入输出---------------------------待写**

##### 5.10.1 print



##### 5.10.2 scan
