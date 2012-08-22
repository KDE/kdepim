set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
  ${CMAKE_BINARY_DIR}/libksieve
  ${CMAKE_SOURCE_DIR}/libksieve
)

########### next target ###############
set(lexertest_SRCS lexertest.cpp)

kde4_add_unit_test(lexertest TESTNAME ksieve-lexertest ${lexertest_SRCS})

target_link_libraries(lexertest ksieve ${KDE4_KDECORE_LIBS})

########### next target ###############

set(parsertest_SRCS parsertest.cpp)

kde4_add_unit_test(parsertest TESTNAME ksieve-parsertest ${parsertest_SRCS})

target_link_libraries(parsertest ksieve ${KDE4_KDECORE_LIBS})
