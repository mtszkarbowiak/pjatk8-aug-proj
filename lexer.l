%{
#include "parser.tab.h"  // Ensure this matches the output from Bison

#define YY_NO_UNISTD_H

#include <io.h>
#define isatty _isatty

%}

%%

[0-9]+  { return NUMBER; }
[a-zA-Z_][a-zA-Z0-9_]*  { return IDENTIFIER; }
"+"     { return PLUS; }
"-"     { return MINUS; }
"*"     { return MULTIPLY; }
"/"     { return DIVIDE; }
"\n"    { return EOL; }
[ \t]   { /* ignore whitespace */ }
.       { return yytext[0]; }

%%

int yywrap() {
    return 1;
}
