
add_subdirectory( pics ) 

include_directories( ${CMAKE_SOURCE_DIR}/ktnef  )


########### next target ###############

set(ktnef_bin_SRCS 
   ktnefmain.cpp 
   ktnefview.cpp 
   attachpropertydialog.cpp 
   messagepropertydialog.cpp 
   main.cpp )

kde4_automoc(${ktnef_bin_SRCS})

kde4_add_ui3_files(ktnef_bin_SRCS attachpropertydialogbase.ui )

kde4_add_executable(ktnef_bin ${ktnef_bin_SRCS})
set_target_properties(ktnef_bin PROPERTIES OUTPUT_NAME ktnef)

target_link_libraries(ktnef_bin  ${KDE4_KDEUI_LIBS} ktnef )

install(TARGETS ktnef_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES ktnef.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES ktnefui.rc  DESTINATION ${DATA_INSTALL_DIR}/ktnef)
install( FILES ms-tnef.desktop  DESTINATION ${MIME_INSTALL_DIR}/application)




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
