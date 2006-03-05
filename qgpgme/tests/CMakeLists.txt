kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(dataprovidertest_SRCS dataprovidertest.cpp )

kde4_automoc(${dataprovidertest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(dataprovidertest ${dataprovidertest_SRCS})

target_link_libraries(dataprovidertest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) $(GPGME_CFLAGS) $(all_includes)
#
#check_PROGRAMS = dataprovidertest
#
#TESTS = $(check_PROGRAMS)
#
#dataprovidertest_SOURCES = dataprovidertest.cpp
#
#LDADD = ../libqgpgme.la #?$(LIB_QT)
