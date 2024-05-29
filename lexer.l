%{

#include "parser.tab.h"  // Ensure this matches the output from Bison

#ifdef _WIN32
    #define YY_NO_UNISTD_H
    #include <io.h>
    #define isatty _isatty
#else
    #include <unistd.h>
#endif

#include "lexing.h"

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

"stop"          { return lu().feed(STOP); }
"let"           { return lu().feed(LET); }
"assert"        { return lu().feed(ASSERT); }

"if"            { return lu().feed(IF); }
"else"          { return lu().feed(ELSE); }
"while"         { return lu().feed(WHILE); }

[0-9]+          { yylval.ival = atoi(yytext); return lu().feed(NUMBER, yytext); }
"true"          { yylval.bval = true; return lu().feed(TRUE); }
"false"         { yylval.bval = false; return lu().feed(FALSE); }

"*"             { return lu().feed(MULTIPLY); }
"/"             { return lu().feed(DIVIDE); }
"%"             { return lu().feed(MODULO); }
"+"             { return lu().feed(PLUS); }
"-"             { return lu().feed(MINUS); }

"=="            { return lu().feed(EQUAL); }
"!="            { return lu().feed(NOT_EQUAL); }
"<"             { return lu().feed(LESS_THAN); }
">"             { return lu().feed(MORE_THAN); }
"<="            { return lu().feed(LESS_EQUAL); }
">="            { return lu().feed(MORE_EQUAL); }

("&&"|"and")    { return lu().feed(LOGIC_AND); }
("||"|"or")     { return lu().feed(LOGIC_OR); }
("^"|"xor")     { return lu().feed(LOGIC_XOR); }

[a-zA-Z_][a-zA-Z0-9_]*  { return lu().feed(IDENTIFIER, yytext); }

"="             { return lu().feed(ASSIGN); }
":"             { return lu().feed(OF_TYPE); }
";"             { return lu().feed(STATEMENT_SEPARATOR); }
"{"             { return lu().feed(BODY_OPEN); }
"}"             { return lu().feed(BODY_CLOSE); }

"\n"            { return lu().feed(YYEOF); }
[ \t]           { /* ignore whitespace */ }
.               { return yytext[0]; }

%%

int yywrap() {
    return 1;
}
