set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
  ${CMAKE_BINARY_DIR}/libksieve
  ${CMAKE_SOURCE_DIR}/libksieve
)

ecm_add_tests( lexertest.cpp parsertest.cpp  NAME_PREFIX "libksieve-parser-" LINK_LIBRARIES ksieve Qt5::Test)
