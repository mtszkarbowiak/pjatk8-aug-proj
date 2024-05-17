#pragma once


// CPP Includes
#include <iostream>

// C Includes
#include <stdio.h>

// BISON Includes
#include "parser.hpp"
#include "parser.tab.h"



template<typename T>
auto raportToken(T token) -> T
{
	std::cout << "Token: " << token << '\n';
	return token;
}
