kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libksieve ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(lexertest_SRCS lexertest.cpp )

kde4_automoc(${lexertest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(lexertest ${lexertest_SRCS})

target_link_libraries(lexertest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(parsertest_SRCS parsertest.cpp )

kde4_automoc(${parsertest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(parsertest ${parsertest_SRCS})

target_link_libraries(parsertest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES = -I$(top_srcdir)/libksieve $(all_includes)
#LDADD = ../libksieve.la
#
## test programs:
#check_PROGRAMS = \
#	lexertest \
#	parsertest
#
#TESTS = $(check_PROGRAMS)
#
#lexertest_SOURCES = lexertest.cpp
#parsertest_SOURCES = parsertest.cpp
