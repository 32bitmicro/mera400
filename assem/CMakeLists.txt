CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(assem C)

set(ASSEM_MAJOR_VERSION 0)
set(ASSEM_MINOR_VERSION 6)
set(ASSEM_PATCH_VERSION 0)
set(ASSEM_VERSION ${ASSEM_MAJOR_VERSION}.${ASSEM_MINOR_VERSION}.${ASSEM_PATCH_VERSION})

find_package(BISON)
find_package(FLEX)

ADD_DEFINITIONS(-Wall -std=c99 -D_XOPEN_SOURCE=700 -O3 -pthread -DASSEM_VERSION="${ASSEM_VERSION}" -g)

BISON_TARGET(parser_modern parser_modern.y ${CMAKE_CURRENT_BINARY_DIR}/parser_modern.c COMPILE_FLAGS "-p m_yy")
BISON_TARGET(parser_classic parser_classic.y ${CMAKE_CURRENT_BINARY_DIR}/parser_classic.c COMPILE_FLAGS "-p c_yy")
FLEX_TARGET(scanner_modern scanner_modern.l ${CMAKE_CURRENT_BINARY_DIR}/scanner_modern.c COMPILE_FLAGS "-P m_yy")
FLEX_TARGET(scanner_classic scanner_classic.l ${CMAKE_CURRENT_BINARY_DIR}/scanner_classic.c COMPILE_FLAGS "-P c_yy")
ADD_FLEX_BISON_DEPENDENCY(scanner_modern parser_modern)
ADD_FLEX_BISON_DEPENDENCY(scanner_classic parser_classic)

ADD_EXECUTABLE(assem main.c elements.c eval.c ops.c pragmas.c
pragmas_classic.c pragmas_modern.c
${BISON_parser_modern_OUTPUTS}
${BISON_parser_classic_OUTPUTS}
${FLEX_scanner_modern_OUTPUTS}
${FLEX_scanner_classic_OUTPUTS}
)

# vim: tabstop=4