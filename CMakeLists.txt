cmake_minimum_required(VERSION 3.16)
project(program_flow_analysis C)

set(CMAKE_C_STANDARD 11)

option(Debug "Debug" OFF)
if (Debug)
        set(BISON_CLI_FLAGS "--debug")
endif (Debug)

find_package(BISON)
find_package(FLEX)

bison_target(parser src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c
            COMPILE_FLAGS ${BISON_CLI_FLAGS})

flex_target(lexer src/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)

add_flex_bison_dependency(lexer parser)

set(SOURCE_FILES
        src/main.c
        src/ast.c
        src/value.c
        src/symtab.c
        src/analyzer.c
        ${BISON_parser_OUTPUT_SOURCE}
        ${FLEX_lexer_OUTPUTS})

set(HEADER_FILES
        src/ast.h
        src/value.h
        src/symtab.h
        src/analyzer.h
        ${BISON_parser_OUTPUT_HEADER})

add_executable(analyzer ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(analyzer PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(analyzer m)
