# CMakeList.txt : CMake project for Cesserract, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if(POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("Cesserract")

# Find Flex and Bison
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# Define the Flex and Bison files
set(FLEX_INPUT ${CMAKE_SOURCE_DIR}/lexer.l)
set(BISON_INPUT ${CMAKE_SOURCE_DIR}/parser.y)

# Generate the lexer and parser
FLEX_TARGET(MyScanner ${FLEX_INPUT} ${CMAKE_BINARY_DIR}/lexer.cpp)
BISON_TARGET(MyParser ${BISON_INPUT} ${CMAKE_BINARY_DIR}/parser.cpp COMPILE_FLAGS "--defines=${CMAKE_BINARY_DIR}/parser.tab.h")

# Include directories for generated files
include_directories(${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR})

# Add source to this project's executable.
add_executable(Cesserract "lexing.cpp" "lexing.h" ${FLEX_MyScanner_OUTPUTS} ${BISON_MyParser_OUTPUTS} "ast.h" "ast.cpp")

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Cesserract PROPERTY CXX_STANDARD 20)
endif()

# Ensure Flex and Bison dependencies are built before the executable
add_flex_bison_dependency(MyScanner MyParser)

# TODO: Add tests and install targets if needed.
