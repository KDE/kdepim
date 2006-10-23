
include_directories(
	${CMAKE_SOURCE_DIR}/libksieve
	
)


########### next target ###############
if(KDE4_BUILD_TESTS)
set(lexertest_SRCS lexertest.cpp )

kde4_automoc(${lexertest_SRCS})

kde4_add_executable(lexertest ${lexertest_SRCS})

target_link_libraries(lexertest  ksieve ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(parsertest_SRCS parsertest.cpp )

kde4_automoc(${parsertest_SRCS})


kde4_add_executable(parsertest ${parsertest_SRCS})

target_link_libraries(parsertest  ksieve ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

