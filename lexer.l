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

%x COMMENT

%%

<INITIAL,COMMENT>"/*" { 
    lu().increase_comment_level(); 
    BEGIN(COMMENT);
}
<COMMENT>"*/" { 
    lu().decrease_comment_level(); 

    if (lu().get_comment_level() == 0) {
		BEGIN(INITIAL);
	}
}
<COMMENT>.|\n  { /* Consume characters inside comments */ }

"stop"      { return lu().feed(STOP); }
"let"       { return lu().feed(LET); }
"assert"    { return lu().feed(ASSERT); }

"if"        { return lu().feed(IF); }
"else"      { return lu().feed(ELSE); }
"while"     { return lu().feed(WHILE); }

[0-9]+      { return lu().feed(NUMBER); }
"true"      { return lu().feed(TRUE); }
"false"     { return lu().feed(FALSE); }

[a-zA-Z_][a-zA-Z0-9_]*  { return lu().feed(IDENTIFIER); }

"*"         { return lu().feed(MULTIPLY); }
"/"         { return lu().feed(DIVIDE); }
"%"         { return lu().feed(MODULO); }
"+"         { return lu().feed(PLUS); }
"-"         { return lu().feed(MINUS); }

"="         { return lu().feed(EQUAL_TO); }
":"         { return lu().feed(OF_TYPE); }

"\n"        { return lu().feed(YYEOF); }
[ \t]       { /* ignore whitespace */ }
.           { return yytext[0]; }

%%

int yywrap() {
    return 1;
}
