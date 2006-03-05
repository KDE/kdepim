kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(testidmapper_SRCS testidmapper.cpp )

kde4_automoc(${testidmapper_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testidmapper ${testidmapper_SRCS})

target_link_libraries(testidmapper  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testemail_SRCS testemail.cpp )

kde4_automoc(${testemail_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testemail ${testemail_SRCS})

target_link_libraries(testemail  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -I$(top_srcdir)/libemailfunctions $(all_includes)
#LDADD = ../libemailfunctions.la $(LIB_KDECORE)
#
#check_PROGRAMS = testidmapper testemail
#TESTS = testidmapper testemail
#
#testidmapper_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testidmapper_SOURCES = testidmapper.cpp
#
#testemail_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testemail_SOURCES = testemail.cpp
#
#METASOURCES = AUTO
