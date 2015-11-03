set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

ecm_add_tests( lexertest.cpp parsertest.cpp  NAME_PREFIX "libksieve-parser-" LINK_LIBRARIES KF5::KSieve Qt5::Test)
