
#include "cesserract.h"

int main()
{
    std::cout << "Hello World!\n";
    return yyparse();
}

void yyerror(const char* s)
{
    std::cout << "Error: " << s << '\n';
}