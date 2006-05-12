
if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/..   )

########### next target ###############

set(testemail_SRCS ${libemailfunctions_SRCS} testemail.cpp )

kde4_automoc(${testemail_SRCS})

kde4_add_executable(testemail ${testemail_SRCS})

target_link_libraries(testemail  ${KDE4_KDECORE_LIBS} )

########### install files ###############

endif(KDE4_BUILD_TESTS)




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
