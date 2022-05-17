%option yylineno
%{
    #include "ast.h"
    #include "gramGen.hpp"
    #include <string.h>
    int mistakeNum = 0; // the number of mistakes
%}
/*key words*/
ELSE else
IF if
RETURN return
WHILE while
BREAK break
TYPE "int"|"float"|"char"|"bool"
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

/*token传入astNode的构造函数*/
%%
{ELSE}  {yylval.asTree = new astNode(yytext, "ELSE", 0); return ELSE;}
{IF}    {yylval.asTree = new astNode(yytext, "IF", 0); return IF;}
{RETURN}    {yylval.asTree = new astNode(yytext, "RETURN", 0); return RETURN;}
{WHILE} {yylval.asTree = new astNode(yytext, "WHILE", 0); return WHILE;}
{BREAK} {yylval.asTree = new astNode(yytext, "BREAK", 0); return BREAK;}
{TYPE}  {yylval.asTree = new astNode(yytext, "TYPE", 0); return TYPE;}
{BOOL}   {yylval.asTree = new astNode(yytext, "BOOL", 0); return BOOL;}

{INT}   {yylval.asTree = new astNode(yytext, "INT", 0); return INT;}
{FLOAT} {yylval.asTree = new astNode(yytext, "FLOAT", 0); return FLOAT;}
{CHAR}  {yylval.asTree = new astNode(yytext, "CHAR", 0); return CHAR;}
{STR}   {yylval.asTree = new astNode(yytext, "STR", 0); return STR;}
{ID}    {yylval.asTree = new astNode(yytext, "ID", 0); return ID;}
{COMMA} {yylval.asTree = new astNode(yytext, "COMMA", 0); return COMMA;}
{SEMICOLON} {yylval.asTree = new astNode(yytext, "SEMICOLON", 0); return SEMICOLON;}
{NEWLINE}   {}
{TAB}   {}
{CR}    {}

{PLUS}  {yylval.asTree = new astNode(yytext, "PLUS", 0); return PLUS;}
{MINUS} {yylval.asTree = new astNode(yytext, "MINUS", 0); return MINUS;}
{MULTI} {yylval.asTree = new astNode(yytext, "MULTI", 0); return MULTI;}
{DIV}   {yylval.asTree = new astNode(yytext, "DIV", 0); return DIV;}
{RELOP}  {yylval.asTree = new astNode(yytext, "RELOP", 0); return RELOP;}
{ASSIGN}    {yylval.asTree = new astNode(yytext, "ASSIGN", 0); return ASSIGN;}
{LOGIC}   {yylval.asTree = new astNode(yytext, "LOGIC", 0); return LOGIC;}
{NOT}   {yylval.asTree = new astNode(yytext, "NOT", 0); return NOT;}
{LPT}   {yylval.asTree = new astNode(yytext, "LPT", 0); return LPT;}
{RPT}   {yylval.asTree = new astNode(yytext, "RPT", 0); return RPT;}
{LSB}   {yylval.asTree = new astNode(yytext, "LSB", 0); return LSB;}
{RSB}   {yylval.asTree = new astNode(yytext, "RSB", 0); return RSB;}
{LCB}   {yylval.asTree = new astNode(yytext, "LCB", 0); return LCB;}
{RCB}   {yylval.asTree = new astNode(yytext, "RCB", 0); return RCB;}
{COMMENT}   {}

.   {
        mistakeNum ++;
        printf("[Error] Illegal Chars Input [%s] Line [%d]\n", yytext, yylineno);
    }
%%
int yywrap(void) {
    return 1;
}