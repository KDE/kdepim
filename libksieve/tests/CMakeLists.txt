
include_directories(
	${CMAKE_SOURCE_DIR}/libksieve
	
)


########### next target ###############
set(lexertest_SRCS lexertest.cpp )


kde4_add_test_executable(lexertest ${lexertest_SRCS})

target_link_libraries(lexertest  ksieve ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(parsertest_SRCS parsertest.cpp )



kde4_add_test_executable(parsertest ${parsertest_SRCS})

target_link_libraries(parsertest  ksieve ${KDE4_KDECORE_LIBS} ${KDE4_KDE3SUPPORT_LIBS})

