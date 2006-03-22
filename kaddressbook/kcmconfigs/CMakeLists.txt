kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kaddressbook ${CMAKE_SOURCE_DIR}/kaddressbook/common ${CMAKE_SOURCE_DIR}/kaddressbook/interfaces ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_kabconfig_PART_SRCS 
   kcmkabconfig.cpp 
   kabconfigwidget.cpp 
   extensionconfigdialog.cpp 
   addresseewidget.cpp )

kde4_automoc(${kcm_kabconfig_PART_SRCS})

kde4_add_plugin(kcm_kabconfig ${kcm_kabconfig_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kabconfig )

target_link_libraries(kcm_kabconfig  ${KDE4_KDECORE_LIBS} kio kabinterfaces kdepim kabcommon )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kabconfig )


########### next target ###############

set(kcm_kabldapconfig_PART_SRCS addhostdialog.cpp kcmkabldapconfig.cpp ldapoptionswidget.cpp )

kde4_automoc(${kcm_kabldapconfig_PART_SRCS})

kde4_add_plugin(kcm_kabldapconfig ${kcm_kabldapconfig_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kabldapconfig )

target_link_libraries(kcm_kabldapconfig  ${KDE4_KDECORE_LIBS} kio kdepim )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kabldapconfig )


########### next target ###############

set(kcm_kabcustomfields_PART_SRCS kcmkabcustomfields.cpp )

kde4_automoc(${kcm_kabcustomfields_PART_SRCS})

kde4_add_plugin(kcm_kabcustomfields ${kcm_kabcustomfields_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kabcustomfields )

target_link_libraries(kcm_kabcustomfields  ${KDE4_KDECORE_LIBS} kdeui kabcommon )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kabcustomfields )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kabconfig.desktop kabldapconfig.desktop                      kabcustomfields.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= -I$(top_builddir)/kaddressbook -I$(top_srcdir)/kaddressbook \
#          -I$(top_srcdir)/kaddressbook/common \
#          -I$(top_srcdir)/kaddressbook/interfaces \
#          -I$(top_srcdir) -I$(top_builddir)/kaddressbook/common $(all_includes)
#
#kde_module_LTLIBRARIES = kcm_kabconfig.la kcm_kabldapconfig.la \
#                         kcm_kabcustomfields.la
#
#kcm_kabconfig_la_SOURCES = kcmkabconfig.cpp kabconfigwidget.cpp \
#                           extensionconfigdialog.cpp addresseewidget.cpp
#kcm_kabconfig_la_LDFLAGS  = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
#                            -avoid-version -no-undefined
#kcm_kabconfig_la_LIBADD = $(LIB_KIO) \
#                          $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
#                          $(top_builddir)/libkdepim/libkdepim.la \
#                          $(top_builddir)/kaddressbook/common/libkabcommon.la
#kcm_kabconfig_la_COMPILE_FIRST = $(top_builddir)/kaddressbook/common/kabprefs_base.h
#
#kcm_kabldapconfig_la_SOURCES = addhostdialog.cpp kcmkabldapconfig.cpp \
#                               ldapoptionswidget.cpp
#kcm_kabldapconfig_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
#                               -avoid-version -no-undefined
#kcm_kabldapconfig_la_LIBADD = $(LIB_KIO) $(LIB_KABC) $(top_builddir)/libkdepim/libkdepim.la
#
#kcm_kabcustomfields_la_SOURCES = kcmkabcustomfields.cpp
#kcm_kabcustomfields_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
#                               -avoid-version -no-undefined
#kcm_kabcustomfields_la_LIBADD = $(LIB_KDEUI) $(LIB_QTDESIGNER) \
#                                $(top_builddir)/kaddressbook/common/libkabcommon.la
#kcm_kabcustomfields_la_COMPILE_FIRST = $(top_builddir)/kaddressbook/common/kabprefs_base.h
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmkabconfig.pot
#
#kde_services_DATA = kabconfig.desktop kabldapconfig.desktop \
#                    kabcustomfields.desktop
