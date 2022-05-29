%option yylineno
%{
    #include "ast.h"
    #include "gramGen.hpp"
    #include <string.h>
%}
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

/*token传入astNode的构造函数*/
%%
{ELSE}  {yylval.asTree = new astNode(yytext, "ELSE"); return ELSE;}
{IF}    {yylval.asTree = new astNode(yytext, "IF"); return IF;}
{RETURN}    {yylval.asTree = new astNode(yytext, "RETURN"); return RETURN;}
{WHILE} {yylval.asTree = new astNode(yytext, "WHILE"); return WHILE;}
{BREAK} {yylval.asTree = new astNode(yytext, "BREAK"); return BREAK;}
{TYPE}  {yylval.asTree = new astNode(yytext, "TYPE"); return TYPE;}
{BOOL}   {yylval.asTree = new astNode(yytext, "BOOL"); return BOOL;}

{INT}   {yylval.asTree = new astNode(yytext, "INT"); return INT;}
{FLOAT} {yylval.asTree = new astNode(yytext, "FLOAT"); return FLOAT;}
{CHAR}  {yylval.asTree = new astNode(yytext, "CHAR"); return CHAR;}
{STR}   {yylval.asTree = new astNode(yytext, "STR"); return STR;}
{ID}    {yylval.asTree = new astNode(yytext, "ID"); return ID;}
{COMMA} {yylval.asTree = new astNode(yytext, "COMMA"); return COMMA;}
{SEMICOLON} {yylval.asTree = new astNode(yytext, "SEMICOLON"); return SEMICOLON;}
{NEWLINE}   {}
{TAB}   {}
{CR}    {}
{BLANK} {}

{PLUS}  {yylval.asTree = new astNode(yytext, "PLUS"); return PLUS;}
{MINUS} {yylval.asTree = new astNode(yytext, "MINUS"); return MINUS;}
{MULTI} {yylval.asTree = new astNode(yytext, "MULTI"); return MULTI;}
{DIV}   {yylval.asTree = new astNode(yytext, "DIV"); return DIV;}
{RELOP}  {yylval.asTree = new astNode(yytext, "RELOP"); return RELOP;}
{ASSIGN}    {yylval.asTree = new astNode(yytext, "ASSIGN"); return ASSIGN;}
{LOGIC}   {yylval.asTree = new astNode(yytext, "LOGIC"); return LOGIC;}
{NOT}   {yylval.asTree = new astNode(yytext, "NOT"); return NOT;}
{LPT}   {yylval.asTree = new astNode(yytext, "LPT"); return LPT;}
{RPT}   {yylval.asTree = new astNode(yytext, "RPT"); return RPT;}
{LSB}   {yylval.asTree = new astNode(yytext, "LSB"); return LSB;}
{RSB}   {yylval.asTree = new astNode(yytext, "RSB"); return RSB;}
{LCB}   {yylval.asTree = new astNode(yytext, "LCB"); return LCB;}
{RCB}   {yylval.asTree = new astNode(yytext, "RCB"); return RCB;}
{COMMENT}   {}

.   {
        printf("[Error] Illegal Chars Input [%s] Line [%d]\n", yytext, yylineno);
    }
%%
int yywrap(void) {
    return 1;
}