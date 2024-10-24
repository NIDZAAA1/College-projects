%{
#include "parser.tab.h"
#include <string.h>
#include "./inc/assembler.h"
%}

%%

\n      { return EOL; }
[ \t]   { /* ignore whitespace */ }
","     { return COMMA; }
"["     { return LBRACKET; }
"]"     { return RBRACKET; }
"+"     { return PLUS; }
"$"     { return DOLLAR; }
"#".*   { /* ignore comments starting with # */ }


(halt|int|iret|ret) {
  yylval.sval = strdup(yytext);
  return INS_1;
}

(push|pop|not) {
  yylval.sval = strdup(yytext);
  return INS_2;
}

(xchg|add|sub|mul|div|and|or|xor|shl|shr) {
  yylval.sval = strdup(yytext);
  return INS_3;
}

(call|jmp) {
  yylval.sval = strdup(yytext);
  return INS_4;
}

(beq|bne|bgt) {
  yylval.sval = strdup(yytext);
  return INS_5;
}

(st) {
  yylval.sval = strdup(yytext);
  return INS_6;
}

(csrrd) {
  yylval.sval = strdup(yytext);
  return INS_7;
}

(csrwr) {
  yylval.sval = strdup(yytext);
  return INS_8;
}

(ld) {
  yylval.sval = strdup(yytext);
  return INS_9;
}


(0[xX][0-9a-fA-F]+|[0-9]+) {
  yylval.sval = strdup(yytext);
  return LITERAL;
}


^[a-zA-Z0-9_][a-zA-Z0-9_]*: { 
    yylval.sval = strdup(yytext);
    yylval.sval[strlen(yylval.sval) - 1] = '\0';
    return LABEL;
}

(.global|.extern) {
  yylval.sval = strdup(yytext);
  return DIR_1;
}

(.section) {
  yylval.sval = strdup(yytext);
  return DIR_2;
}

(.word) {
  yylval.sval = strdup(yytext);
  return DIR_3;
}

(.skip) {
  yylval.sval = strdup(yytext);
  return DIR_4;
}

(.end) {
  yylval.sval = strdup(yytext);
  return DIR_7;
}


[\.a-zA-Z_0-9][a-zA-Z_0-9]* {
  yylval.sval = strdup(yytext);
  return SYMBOL;
}


%r([0-9]|1[0-5]) {
  yylval.sval = strdup(yytext + 2);
  return GPR;
}

(%sp) {
  yylval.sval = strdup("14");
  return GPR;
}

(%pc) {
  yylval.sval = strdup("15");
  return GPR;
}

(%status|%handler|%cause) {
  yylval.sval = strdup(yytext+1);
  return CSR;
}

%%
