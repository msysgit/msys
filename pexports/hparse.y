%{
#include <string.h>
#include <stdio.h>

#include "pexports.h"


static int inc_flag = 1;
static int arg_size = 0;

extern int yylex(void);

static void yyerror(char *s);

#define INC_ARGS(n) arg_size += (inc_flag ? n : 0)
%}

%union {
       char *s;
};

%token <s> ID

%type <s> type_name
%%

start
        : function_declaration
        | start function_declaration
        ;

function_declaration
        : type_name '(' parameter_list ')' ';'
        {
          ADD_FUNCTION($1, arg_size);
          arg_size = 0;
        }
        | type_name '(' ')' ';'
        {
          ADD_FUNCTION($1, 0);
          arg_size = 0;
        }
        | error { arg_size = 0; yyclearin; }
        ;

type_name
        : ID
        | ID pointer
        { $$ = ""; }
        | type_name ID
        { $$ = $2; }
        | type_name ID pointer
        { $$ = ""; }
        | type_name function_pointer
        { $$ = ""; }
        ;

function_pointer
        : '(' function_pointer_name ')' '(' ')'
        {}
        | '(' function_pointer_name ')' '(' 
        { inc_flag = 0; }
        parameter_list ')'
        { inc_flag = 1; }
        ;

function_pointer_name
        : pointer
        | pointer ID
        ;

pointer
        : '*'
        | '*' pointer
        ;

parameter_declaration
        : type_name
        {
          if (strcmp($1, "POINT") == 0)
            INC_ARGS(8);
          else if (strcmp($1, "RECT") == 0)
            INC_ARGS(16);
          else if (strcmp($1, "float") == 0)
            INC_ARGS(sizeof(float));
          else if (strcmp($1, "double") == 0)
            INC_ARGS(sizeof(double));
          else if (strcmp($1, "void") != 0)
            INC_ARGS(4);
        }
        ;

parameter_list
        : parameter_declaration
        | parameter_list ',' parameter_declaration
        ;

%%
static void
yyerror(char *s)
{
  /* ignore error */
  arg_size = 0;
}
