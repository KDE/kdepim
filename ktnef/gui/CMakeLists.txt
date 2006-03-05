kde4_header()

add_subdirectory( pics ) 

include_directories( ${CMAKE_SOURCE_DIR}/ktnef ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(ktnef_SRCS 
   ktnefmain.cpp 
   ktnefview.cpp 
   attachpropertydialog.cpp 
   messagepropertydialog.cpp 
   main.cpp )

kde4_automoc(${ktnef_SRCS})

kde4_add_ui_files(ktnef_SRCS attachpropertydialogbase.ui )

kde4_add_executable(ktnef ${ktnef_SRCS})

target_link_libraries(ktnef  ${QT_AND_KDECORE_LIBS} ktnef )

install_targets(/bin ktnef )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES ktnef.desktop )
install_files( ${DATA_INSTALL_DIR}/ktnef FILES ktnefui.rc )
install_files( ${MIME_INSTALL_DIR}/application FILES ms-tnef.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/ktnef $(all_includes)
#
#SUBDIRS = pics
#
#bin_PROGRAMS = ktnef
#
#ktnef_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#ktnef_LDADD   = $(LIB_KFILE) $(top_builddir)/ktnef/lib/libktnef.la
#ktnef_SOURCES = ktnefmain.cpp ktnefview.cpp attachpropertydialogbase.ui \
#                attachpropertydialog.cpp messagepropertydialog.cpp \
#                main.cpp
#                #qwmf.cpp main.cpp
#ktnef_METASOURCES = AUTO
#
#noinst_HEADERS = ktnefmain.h ktnefview.h attachpropertydialog.h \
#                 qwmf.h wmfstruct.h metafuncs.h
#
#rcdir = $(kde_datadir)/ktnef
#rc_DATA = ktnefui.rc
#
##messages: rc.cpp
##	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(ktnef_SOURCES) && mv messages.po ../po/ktnef.pot
#
#xdg_apps_DATA = ktnef.desktop
#
#mime_DATA = ms-tnef.desktop
#mimedir = $(kde_mimedir)/application
