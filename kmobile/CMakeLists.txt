kde4_header()

add_subdirectory( devices ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kmobile_SRCS 
   main.cpp 
   kmobile.cpp 
   kmobileview.cpp 
   kmobileitem.cpp 
   pref.cpp 
   systemtray.cpp )

kde4_automoc(${kmobile_SRCS})

kde4_add_ui_files(kmobile_SRCS kmobile_selectiondialog.ui )

kde4_add_dcop_skels(kmobile_SRCS kmobileiface.h )

kde4_add_executable(kmobile ${kmobile_SRCS})

target_link_libraries(kmobile  ${QT_AND_KDECORE_LIBS} kmobiledevice )

install_targets(/bin kmobile )


########### next target ###############

set(kmobiledevice_LIB_SRCS kmobiledevice.cpp )

kde4_automoc(${kmobiledevice_LIB_SRCS})

kde4_add_library(kmobiledevice SHARED ${kmobiledevice_LIB_SRCS})

target_link_libraries(kmobiledevice  ${QT_AND_KDECORE_LIBS} )

set_target_properties(kmobiledevice PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kmobiledevice )


########### next target ###############

set(kmobileclient_LIB_SRCS kmobileclient.cpp )

kde4_automoc(${kmobileclient_LIB_SRCS})

kde4_add_library(kmobileclient SHARED ${kmobileclient_LIB_SRCS})

target_link_libraries(kmobileclient  ${QT_AND_KDECORE_LIBS} DCOP )

set_target_properties(kmobileclient PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kmobileclient )


########### next target ###############

set(testlibkmobile_SRCS testlibkmobile.cpp )

kde4_automoc(${testlibkmobile_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testlibkmobile ${testlibkmobile_SRCS})

target_link_libraries(testlibkmobile  ${QT_AND_KDECORE_LIBS} kmobileclient )

endif(KDE4_BUILD_TESTS)

########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kmobile.desktop )
install_files( ${SERVICETYPES_INSTALL_DIR} FILES libkmobile.desktop )
install_files( ${DATA_INSTALL_DIR}/kmobile FILES kmobileui.rc )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for kmobile
#
#SUBDIRS = . devices		# kioslave
#
#bin_PROGRAMS = kmobile 
#
#lib_LTLIBRARIES = libkmobiledevice.la libkmobileclient.la
#
#INCLUDES         = $(all_includes)
#
#kmobile_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#kmobile_LDADD   = $(LIB_KFILE) $(LIB_KABC) libkmobiledevice.la
#kmobile_SOURCES = main.cpp kmobile.cpp kmobileview.cpp kmobileitem.cpp\
#			pref.cpp kmobileiface.skel kmobile_selectiondialog.ui \
#			systemtray.cpp
#
#KDE_ICON = kmobile
#
#noinst_HEADERS = kmobile.h kmobileview.h kmobileitem.h pref.h
#
## the low-level devices driver library
#libkmobiledevice_la_SOURCES = kmobiledevice.cpp
#libkmobiledevice_la_LDFLAGS = $(all_libraries) -no-undefined -avoid-version
#libkmobiledevice_la_LIBADD = $(LIB_KABC) $(LIB_LOCKDEV) #../libkcal/libkcal.la
#
## the KDE application's client library to KMobile
#libkmobileclient_la_SOURCES = kmobileclient.cpp
#libkmobileclient_la_LDFLAGS = $(all_libraries) -no-undefined -avoid-version
#libkmobileclient_la_LIBADD = $(LIB_KDECORE) -lDCOP
#
#EXTRA_PROGRAMS = testlibkmobile
#testlibkmobile_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testlibkmobile_LDADD = libkmobileclient.la
#testlibkmobile_SOURCES = testlibkmobile.cpp
#
## the servicetype for mobile devices
#kde_servicetypes_DATA = libkmobile.desktop
#
#METASOURCES = AUTO
#
#xdg_apps_DATA = kmobile.desktop
#
#rc_DATA = kmobileui.rc
#rcdir = $(kde_datadir)/kmobile
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kmobile.pot
#
