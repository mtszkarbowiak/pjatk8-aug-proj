%{

#include "parser.tab.h"  // Ensure this matches the output from Bison

#ifdef _WIN32
    #define YY_NO_UNISTD_H
    #include <io.h>
    #define isatty _isatty
#else
    #include <unistd.h>
#endif

#include "interpreter.h"

%}

%%

[0-9]+      { return feed_token(NUMBER); }
[a-zA-Z_][a-zA-Z0-9_]*  { return feed_token(IDENTIFIER); }

"+"         { return feed_token(PLUS); }
"-"         { return feed_token(MINUS); }
"*"         { return feed_token(MULTIPLY); }
"/"         { return feed_token(DIVIDE); }

"\n"        { return feed_token(EOL); }
[ \t]       { /* ignore whitespace */ }
.           { return yytext[0]; }

%%

int yywrap() {
    return 1;
}
