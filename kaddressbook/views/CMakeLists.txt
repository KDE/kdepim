kde4_header()

include_directories(
	${CMAKE_SOURCE_DIR}/kaddressbook/common
	${CMAKE_SOURCE_DIR}/kaddressbook/interfaces
	${CMAKE_SOURCE_DIR}/kaddressbook
	${CMAKE_BINARY_DIR}/kaddressbook
	${KDE4_INCLUDES}
)


########### next target ###############

set(kaddrbk_cardview_PART_SRCS ${libkabcommon_SRCS} 
   cardview.cpp 
   colorlistbox.cpp 
   configurecardviewdialog.cpp 
   kaddressbookcardview.cpp )

kde4_automoc(${kaddrbk_cardview_PART_SRCS})

kde4_add_plugin(kaddrbk_cardview WITH_PREFIX ${kaddrbk_cardview_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_cardview )

target_link_libraries(kaddrbk_cardview  ${KDE4_KDECORE_LIBS} kaddressbook kabc)

install_targets(${PLUGIN_INSTALL_DIR} kaddrbk_cardview )


########### next target ###############

set(kaddrbk_iconview_PART_SRCS kaddressbookiconview.cpp )

kde4_automoc(${kaddrbk_iconview_PART_SRCS})

kde4_add_plugin(kaddrbk_iconview WITH_PREFIX ${kaddrbk_iconview_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_iconview )

target_link_libraries(kaddrbk_iconview  ${KDE4_KDECORE_LIBS} kabc kaddressbook ${LIB_KDE3SUPPORT})

install_targets(${PLUGIN_INSTALL_DIR} kaddrbk_iconview )


########### next target ###############

set(kaddrbk_tableview_PART_SRCS configuretableviewdialog.cpp contactlistview.cpp kaddressbooktableview.cpp )

kde4_automoc(${kaddrbk_tableview_PART_SRCS})

kde4_add_plugin(kaddrbk_tableview WITH_PREFIX ${kaddrbk_tableview_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_tableview )

target_link_libraries(kaddrbk_tableview  ${KDE4_KDECORE_LIBS} kabc kaddressbook ${LIB_KDE3SUPPORT})

install_targets(${PLUGIN_INSTALL_DIR} kaddrbk_tableview )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kaddressbook FILES cardview.desktop iconview.desktop tableview.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) -I$(top_builddir)/kaddressbook/common \
#           -I$(top_srcdir)/kaddressbook/common \
#           -I$(top_srcdir)/kaddressbook/interfaces \
#           -I$(top_srcdir)/kaddressbook $(all_includes)
#
#kde_module_LTLIBRARIES = libkaddrbk_cardview.la libkaddrbk_iconview.la \
#                         libkaddrbk_tableview.la
#
#XXLIBS = $(top_builddir)/kaddressbook/libkaddressbook.la
#
#libkaddrbk_cardview_la_SOURCES = cardview.cpp colorlistbox.cpp \
#                                 configurecardviewdialog.cpp \
#                                 kaddressbookcardview.cpp
#libkaddrbk_cardview_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
#libkaddrbk_cardview_la_LIBADD = $(XXLIBS)
#
#libkaddrbk_iconview_la_SOURCES = kaddressbookiconview.cpp
#libkaddrbk_iconview_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
#libkaddrbk_iconview_la_LIBADD = $(XXLIBS)
#
#libkaddrbk_tableview_la_SOURCES = configuretableviewdialog.cpp \
#                                  contactlistview.cpp \
#                                  kaddressbooktableview.cpp
#libkaddrbk_tableview_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
#libkaddrbk_tableview_la_LIBADD = $(XXLIBS)
#
#noinst_HEADERS = cardview.h colorlistbox.h configurecardviewdialog.h \
#                 configuretableviewdialog.h contactlistview.h \
#                 kaddressbookcardview.h kaddressbookiconview.h \
#                 kaddressbooktableview.h
#
#METASOURCES = AUTO
#
#servicedir  = $(kde_servicesdir)/kaddressbook
#service_DATA = cardview.desktop iconview.desktop tableview.desktop
