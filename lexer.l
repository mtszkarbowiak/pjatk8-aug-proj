%{

#include "parser.tab.h"  // Ensure this matches the output from Bison

#ifdef _WIN32
    #define YY_NO_UNISTD_H
    #include <io.h>
    #define isatty _isatty
#else
    #include <unistd.h>
#endif

#include "cesserract.h"

%}

%%

[0-9]+      { return raportToken(NUMBER); }
[a-zA-Z_][a-zA-Z0-9_]*  { return raportToken(IDENTIFIER); }

"+"         { return raportToken(PLUS); }
"-"         { return raportToken(MINUS); }
"*"         { return raportToken(MULTIPLY); }
"/"         { return raportToken(DIVIDE); }

"\n"        { return raportToken(EOL); }
[ \t]       { /* ignore whitespace */ }
.           { return yytext[0]; }

%%

int yywrap() {
    return 1;
}
