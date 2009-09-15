D                       [0-9]
L                       [a-zA-Z_]
OS [ ]*

%{
#include <stdio.h>
#include <string.h>
#include "hparse.h"

static void comment(void);

%}

%%
"#".*                   { }
"//".*                  { }
"/*"                    { comment(); }

"__"{L}+                { return(ID); }
"__"{L}+{OS}*"("+{L}({L}|{D})*")"+ { return(ID); }
{L}({L}|{D})*           {
                                yylval.s = strdup(yytext);
                                return ID;
                        }

","                     { return(','); }
"*"                     { return('*'); }
"("                     { return('('); }
")"                     { return(')'); }
";"                     { return(';'); }

[ \t\v\n\f]             { }
.                       { }

%%

int
yywrap(void)
{
        return(1);
}

static void
comment(void)
{
        char c, c1;

loop:
        while ((c = input()) != '*' && c != 0)
                putchar(c);

        if ((c1 = input()) != '/' && c != 0)
        {
                unput(c1);
                goto loop;
        }

        if (c != 0)
                putchar(c1);
}
