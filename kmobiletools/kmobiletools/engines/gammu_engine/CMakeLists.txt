
include_directories( ${CMAKE_SOURCE_DIR}/kmobiletools/kmobiletools/libkmobiletools ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kmobiletools_gammu_LIB_SRCS 
   kmobiletoolsgammu_engine.cpp 
   gammu_jobs.cpp 
   qphoneerror.cpp 
   device.cpp )


kde4_add_library(kmobiletools_gammu SHARED ${kmobiletools_gammu_LIB_SRCS})

target_link_libraries(kmobiletools_gammu  ${KDE4_KDECORE_LIBS} Gammu kmobiletools )

set_target_properties(kmobiletools_gammu PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kmobiletools_gammu DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#KDE_OPTIONS = nofinal
#
#INCLUDES = -I$(top_srcdir)/kmobiletools/kmobiletools/libkmobiletools \
#    -I$(top_builddir)/kmobiletools/kmobiletools/libkmobiletools \
#	$(all_includes)
#METASOURCES = AUTO
#lib_LTLIBRARIES = libkmobiletools_gammu.la
#
#libkmobiletools_gammu_la_SOURCES = kmobiletoolsgammu_engine.cpp gammu_jobs.cpp qphoneerror.cpp \
#				   device.cpp
#
#noinst_HEADERS = kmobiletoolsgammu_engine.h gammu_jobs.h qphoneerror.h device.h memory_jobs.h sms_jobs.h
#
#libkmobiletools_gammu_la_LDFLAGS = -avoid-version -module -no-undefined \
#	$(all_libraries) -lGammu
#
#libkmobiletools_gammu_la_LIBADD =\
#	$(top_builddir)/kmobiletools/kmobiletools/libkmobiletools/libkmobiletools.la
#KDE_CXXFLAGS = -UQT_NO_ASCII_CAST
#
