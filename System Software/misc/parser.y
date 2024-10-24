%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./inc/assembler.h"
#include "parser.tab.h"

ParsedLine parsed_line;

%}

%union {
    char* sval;
    struct {
        char* reg;
        char* offset;
        int info;
    } regoff;
    struct {
        char* val;
        int info;
    } operand;
}

%token <sval> EOL
%token <sval> LABEL
%token <sval> DIR_1
%token <sval> DIR_2
%token <sval> DIR_3
%token <sval> DIR_4
%token <sval> DIR_5
%token <sval> DIR_6
%token <sval> DIR_7
%token <sval> INS_1
%token <sval> INS_2
%token <sval> INS_3
%token <sval> INS_4
%token <sval> INS_5
%token <sval> INS_6
%token <sval> INS_7
%token <sval> INS_8
%token <sval> INS_9
%token <sval> LITERAL
%token <sval> SYMBOL
%token <sval> GPR
%token <sval> CSR
%type <operand> operand
%type <regoff> regoff
%token COMMA
%token PLUS
%token LBRACKET
%token RBRACKET
%token DOLLAR
%%

input:
    | input label_line
    ;

label_line:
    LABEL line EOL {
        parsed_line.label = $1;
    }
    | LABEL EOL {
        parsed_line.label = $1;
    }
    | line EOL
    | EOL { parsed_line.empty = 1; }

line:
    INS_1 {
        parsed_line.mnemonic = $1;
        parsed_line.arg_count = 0;
    }
    | INS_2 GPR {
        parsed_line.mnemonic = $1;
        parsed_line.arguments = malloc(1 * sizeof(char*));
        parsed_line.arguments[0] = strdup($2);
        parsed_line.arg_count = 1;
    }
    | INS_3 GPR COMMA GPR {
        parsed_line.mnemonic = $1;
        parsed_line.arguments = malloc(2 * sizeof(char*));
        parsed_line.arguments[0] = strdup($2);
        parsed_line.arguments[1] = strdup($4);
        parsed_line.arg_count = 2;
    }
    | INS_4 SYMBOL {
        parsed_line.mnemonic = $1;
        parsed_line.arguments = malloc(1 * sizeof(char*));
        parsed_line.arguments[0] = strdup($2);
        parsed_line.arg_count = 1;
        parsed_line.info = 1;
    }
    | INS_4 LITERAL {
        parsed_line.mnemonic = $1;
        parsed_line.arguments = malloc(1 * sizeof(char*));
        parsed_line.arguments[0] = strdup($2);
        parsed_line.arg_count = 1;
        parsed_line.info = 0;
    }
    | INS_5 GPR COMMA GPR COMMA SYMBOL {
        parsed_line.mnemonic = $1;
        parsed_line.arguments = malloc(3 * sizeof(char*));
        parsed_line.arguments[0] = strdup($2);
        parsed_line.arguments[1] = strdup($4);
        parsed_line.arguments[2] = strdup($6);
        parsed_line.arg_count = 3;
    }
    | INS_5 GPR COMMA GPR COMMA LITERAL {
        parsed_line.mnemonic = $1;
        parsed_line.arguments = malloc(3 * sizeof(char*));
        parsed_line.arguments[0] = strdup($2);
        parsed_line.arguments[1] = strdup($4);
        parsed_line.arguments[2] = strdup($6);
        parsed_line.arg_count = 3;
    }
    | INS_6 GPR COMMA operand {
        parsed_line.mnemonic = $1;
        parsed_line.arguments = malloc(2 * sizeof(char*));
        parsed_line.arguments[0] = strdup($2);
        parsed_line.arguments[1] = strdup($4.val);
        parsed_line.arg_count = 2;
        parsed_line.info = $4.info;
    }
    | INS_6 GPR COMMA regoff {
        parsed_line.mnemonic = $1;
        parsed_line.arguments = malloc(3 * sizeof(char*));
        parsed_line.arguments[0] = strdup($2);
        parsed_line.arguments[1] = strdup($4.reg);
        parsed_line.arguments[2] = strdup($4.offset);
        parsed_line.arg_count = 3;
    }
    | INS_7 CSR COMMA GPR {
        parsed_line.mnemonic = $1;
        parsed_line.arguments = malloc(2 * sizeof(char*));
        parsed_line.arguments[0] = strdup($2);
        parsed_line.arguments[1] = strdup($4);
        parsed_line.arg_count = 2;
    }
    | INS_8 GPR COMMA CSR {
        parsed_line.mnemonic = $1;
        parsed_line.arguments = malloc(2 * sizeof(char*));
        parsed_line.arguments[0] = strdup($2);
        parsed_line.arguments[1] = strdup($4);
        parsed_line.arg_count = 2;
    }
    | INS_9 operand COMMA GPR {
        parsed_line.mnemonic = $1;
        parsed_line.arguments = malloc(2 * sizeof(char*));
        parsed_line.arguments[0] = strdup($2.val);
        parsed_line.arguments[1] = strdup($4);
        parsed_line.arg_count = 2;
        parsed_line.info = $2.info;
    }
    | INS_9 regoff COMMA GPR {
        parsed_line.mnemonic = $1;
        parsed_line.arguments = malloc(3 * sizeof(char*));
        parsed_line.arguments[0] = strdup($2.reg);
        parsed_line.arguments[1] = strdup($2.offset);
        parsed_line.arguments[2] = strdup($4);
        parsed_line.arg_count = 3;
        parsed_line.info = $2.info;
    }
    | DIR_1 symbol_list {
        parsed_line.mnemonic = $1;
    }
    | DIR_2 SYMBOL {
        parsed_line.mnemonic = $1;
        parsed_line.arguments = malloc(sizeof(char*));
        parsed_line.arguments[0] = strdup($2);
        parsed_line.arg_count = 1;
    }
    | DIR_3 symbol_literal_list {
        parsed_line.mnemonic = $1;
    }
    | DIR_4 LITERAL {
        parsed_line.mnemonic = $1;
        parsed_line.arguments = malloc(sizeof(char*));
        parsed_line.arguments[0] = strdup($2);
        parsed_line.arg_count = 1;
    }
    | DIR_7 {
        parsed_line.mnemonic = $1;
        parsed_line.arg_count = 0;
    }
    ;

operand:
    DOLLAR LITERAL          { $$.val = $2;$$.info = 0;}
    | DOLLAR SYMBOL         { $$.val = $2;$$.info = 1;}
    | LITERAL               { $$.val = $1;$$.info = 2;}
    | SYMBOL                { $$.val = $1;$$.info = 3;}
    | GPR                   { $$.val = $1;$$.info = 4;}
    | LBRACKET GPR RBRACKET { $$.val = $2;$$.info = 5;}
    ;

regoff:
    LBRACKET GPR PLUS LITERAL RBRACKET { 
        $$.reg = $2;
        $$.offset = $4;
        $$.info = 6;
        }
    | LBRACKET GPR PLUS SYMBOL RBRACKET { 
        $$.reg = $2;
        $$.offset = $4;
        $$.info = 7;
        }
    ;

symbol_list:
    | SYMBOL {
        parsed_line.arguments = malloc(1 * sizeof(char*));
        parsed_line.arguments[0] = strdup($1);
        parsed_line.arg_count = 1;
    }
    | symbol_list COMMA SYMBOL {
        parsed_line.arg_count++;
        parsed_line.arguments = realloc(parsed_line.arguments, parsed_line.arg_count * sizeof(char*));
        parsed_line.arguments[parsed_line.arg_count - 1] = strdup($3);
    }
    ;

symbol_literal_list:
    | SYMBOL {
        parsed_line.arguments = malloc(1 * sizeof(char*));
        parsed_line.arguments[0] = strdup($1);
        parsed_line.arg_count = 1;
        parsed_line.isLiteral = malloc(1*sizeof(int));
        parsed_line.isLiteral[0] = 0;
    }
    | LITERAL {
        parsed_line.arguments = malloc(1 * sizeof(char*));
        parsed_line.arguments[0] = strdup($1);
        parsed_line.arg_count = 1;
        parsed_line.isLiteral = malloc(1*sizeof(int));
        parsed_line.isLiteral[0] = 1;
    }
    | symbol_literal_list COMMA SYMBOL {
        parsed_line.arg_count++;
        parsed_line.arguments = realloc(parsed_line.arguments, parsed_line.arg_count * sizeof(char*));
        parsed_line.arguments[parsed_line.arg_count - 1] = strdup($3);
        parsed_line.isLiteral = realloc(parsed_line.isLiteral, parsed_line.arg_count*sizeof(int));
        parsed_line.isLiteral[parsed_line.arg_count - 1] = 0;

    }
    | symbol_literal_list COMMA LITERAL {
        parsed_line.arg_count++;
        parsed_line.arguments = realloc(parsed_line.arguments, parsed_line.arg_count * sizeof(char*));
        parsed_line.arguments[parsed_line.arg_count - 1] = strdup($3);
        parsed_line.isLiteral = realloc(parsed_line.isLiteral, parsed_line.arg_count*sizeof(int));
        parsed_line.isLiteral[parsed_line.arg_count - 1] = 1;
    }

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
    exit(-1);
}
