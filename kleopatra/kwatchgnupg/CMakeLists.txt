kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libkleo ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kwatchgnupg_SRCS 
   kwatchgnupgmainwin.cpp 
   kwatchgnupgconfig.cpp 
   aboutdata.cpp 
   tray.cpp 
   main.cpp )

kde4_automoc(${kwatchgnupg_SRCS})

kde4_add_executable(kwatchgnupg ${kwatchgnupg_SRCS})

target_link_libraries(kwatchgnupg  ${KDE4_KDECORE_LIBS} conf kleopatra kutils )

install_targets(/bin kwatchgnupg )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kwatchgnupg/pics FILES kwatchgnupg.png kwatchgnupg2.png )
install_files( ${DATA_INSTALL_DIR}/kwatchgnupg FILES kwatchgnupgui.rc )

kde4_footer()



#original Makefile.am contents follow:

#KDE_CXXFLAGS = -DLIBKLEOPATRA_NO_COMPAT
#
#INCLUDES = -I$(top_srcdir)/libkleo  $(GPGME_CFLAGS) $(all_includes)
#
#bin_PROGRAMS = kwatchgnupg
#
#kwatchgnupg_SOURCES = \
#	kwatchgnupgmainwin.cpp \
#	kwatchgnupgconfig.cpp \
#	aboutdata.cpp \
#	tray.cpp \
#	main.cpp
#
#METASOURCES = AUTO
#
#kwatchgnupg_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kwatchgnupg_LDADD   = ../conf/libconf.la ../../libkleo/libkleopatra.la $(LIB_KUTILS) $(LIB_KABC)
#
#rcdir = $(kde_datadir)/kwatchgnupg
#rc_DATA = kwatchgnupgui.rc
#
#
#pics_DATA = kwatchgnupg.png kwatchgnupg2.png
#
#picsdir = $(kde_datadir)/kwatchgnupg/pics
#
#EXTRA_DIST = $(pics_DATA)
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o $(podir)/kwatchgnupg.pot
