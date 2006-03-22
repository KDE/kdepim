kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kaddressbook/interfaces ${CMAKE_SOURCE_DIR}/kaddressbook ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kaddrbk_csv_xxport_PART_SRCS csv_xxport.cpp csvimportdialog.cpp dateparser.cpp )

kde4_automoc(${kaddrbk_csv_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_csv_xxport WITH_PREFIX ${kaddrbk_csv_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_csv_xxport )

target_link_libraries(kaddrbk_csv_xxport  ${KDE4_KDECORE_LIBS} )

install_targets(${LIB_INSTALL_DIR}/kde4 kaddrbk_csv_xxport )


########### next target ###############

set(kaddrbk_vcard_xxport_PART_SRCS vcard_xxport.cpp )

kde4_automoc(${kaddrbk_vcard_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_vcard_xxport WITH_PREFIX ${kaddrbk_vcard_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_vcard_xxport )

target_link_libraries(kaddrbk_vcard_xxport  ${KDE4_KDECORE_LIBS} kaddressbook qgpgme )

install_targets(${LIB_INSTALL_DIR}/kde4 kaddrbk_vcard_xxport )


########### next target ###############

set(kaddrbk_kde2_xxport_PART_SRCS kde2_xxport.cpp )

kde4_automoc(${kaddrbk_kde2_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_kde2_xxport WITH_PREFIX ${kaddrbk_kde2_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_kde2_xxport )

target_link_libraries(kaddrbk_kde2_xxport  ${KDE4_KDECORE_LIBS} )

install_targets(${LIB_INSTALL_DIR}/kde4 kaddrbk_kde2_xxport )


########### next target ###############

set(kaddrbk_bookmark_xxport_PART_SRCS bookmark_xxport.cpp )

kde4_automoc(${kaddrbk_bookmark_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_bookmark_xxport WITH_PREFIX ${kaddrbk_bookmark_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_bookmark_xxport )

target_link_libraries(kaddrbk_bookmark_xxport  ${KDE4_KDECORE_LIBS} )

install_targets(${LIB_INSTALL_DIR}/kde4 kaddrbk_bookmark_xxport )


########### next target ###############

set(kaddrbk_eudora_xxport_PART_SRCS eudora_xxport.cpp )

kde4_automoc(${kaddrbk_eudora_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_eudora_xxport WITH_PREFIX ${kaddrbk_eudora_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_eudora_xxport )

target_link_libraries(kaddrbk_eudora_xxport  ${KDE4_KDECORE_LIBS} )

install_targets(${LIB_INSTALL_DIR}/kde4 kaddrbk_eudora_xxport )


########### next target ###############

set(kaddrbk_ldif_xxport_PART_SRCS ldif_xxport.cpp )

kde4_automoc(${kaddrbk_ldif_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_ldif_xxport WITH_PREFIX ${kaddrbk_ldif_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_ldif_xxport )

target_link_libraries(kaddrbk_ldif_xxport  ${KDE4_KDECORE_LIBS} )

install_targets(${LIB_INSTALL_DIR}/kde4 kaddrbk_ldif_xxport )


########### next target ###############

set(kaddrbk_opera_xxport_PART_SRCS opera_xxport.cpp )

kde4_automoc(${kaddrbk_opera_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_opera_xxport WITH_PREFIX ${kaddrbk_opera_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_opera_xxport )

target_link_libraries(kaddrbk_opera_xxport  ${KDE4_KDECORE_LIBS} )

install_targets(${LIB_INSTALL_DIR}/kde4 kaddrbk_opera_xxport )


########### next target ###############

set(kaddrbk_pab_xxport_PART_SRCS pab_xxport.cpp pab_mapihd.cpp pab_pablib.cpp )

kde4_automoc(${kaddrbk_pab_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_pab_xxport WITH_PREFIX ${kaddrbk_pab_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_pab_xxport )

target_link_libraries(kaddrbk_pab_xxport  ${KDE4_KDECORE_LIBS} )

install_targets(${LIB_INSTALL_DIR}/kde4 kaddrbk_pab_xxport )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kaddressbook FILES csv_xxport.desktop vcard_xxport.desktop kde2_xxport.desktop                 bookmark_xxport.desktop eudora_xxport.desktop ldif_xxport.desktop                 gnokii_xxport.desktop opera_xxport.desktop pab_xxport.desktop )
install_files( ${DATA_INSTALL_DIR}/kaddressbook FILES csv_xxportui.rc vcard_xxportui.rc kde2_xxportui.rc            bookmark_xxportui.rc eudora_xxportui.rc ldif_xxportui.rc            opera_xxportui.rc pab_xxportui.rc $(gnokii_RCFILE) )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) \
#           -I$(top_srcdir)/kaddressbook/interfaces \
#           -I$(top_srcdir)/kaddressbook \
#            \
#           $(GPGME_CFLAGS) \
#           $(all_includes)
#
#if compile_GNOKIIXXPORT
#  gnokii_MODULE = libkaddrbk_gnokii_xxport.la
#  gnokii_RCFILE = gnokii_xxportui.rc
#endif
#
#kde_module_LTLIBRARIES = libkaddrbk_csv_xxport.la libkaddrbk_vcard_xxport.la \
#                         libkaddrbk_kde2_xxport.la libkaddrbk_bookmark_xxport.la \
#                         libkaddrbk_eudora_xxport.la libkaddrbk_ldif_xxport.la \
#                         libkaddrbk_opera_xxport.la libkaddrbk_pab_xxport.la $(gnokii_MODULE)
#
#AM_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
#
#XXLIBS = $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
#         $(top_builddir)/libkdepim/libkdepim.la
#
#libkaddrbk_csv_xxport_la_SOURCES = csv_xxport.cpp csvimportdialog.cpp dateparser.cpp
#libkaddrbk_csv_xxport_la_LIBADD = $(XXLIBS)
#
#libkaddrbk_vcard_xxport_la_SOURCES = vcard_xxport.cpp
#libkaddrbk_vcard_xxport_la_LIBADD = $(XXLIBS) \
#                                    $(top_builddir)/kaddressbook/libkaddressbook.la \
#                                    $(top_builddir)/qgpgme/libqgpgme.la
#
#libkaddrbk_kde2_xxport_la_SOURCES = kde2_xxport.cpp
#libkaddrbk_kde2_xxport_la_LIBADD = $(XXLIBS)
#
#libkaddrbk_bookmark_xxport_la_SOURCES = bookmark_xxport.cpp
#libkaddrbk_bookmark_xxport_la_LIBADD = $(XXLIBS)
#
#libkaddrbk_eudora_xxport_la_SOURCES = eudora_xxport.cpp
#libkaddrbk_eudora_xxport_la_LIBADD = $(XXLIBS)
#
#libkaddrbk_ldif_xxport_la_SOURCES = ldif_xxport.cpp
#libkaddrbk_ldif_xxport_la_LIBADD = $(XXLIBS)
#
#libkaddrbk_gnokii_xxport_la_SOURCES = gnokii_xxport.cpp
#libkaddrbk_gnokii_xxport_la_LIBADD = $(XPMLIB) $(LIB_GNOKII) $(XXLIBS)
#
#libkaddrbk_opera_xxport_la_SOURCES = opera_xxport.cpp
#libkaddrbk_opera_xxport_la_LIBADD = $(XXLIBS)
#
#libkaddrbk_pab_xxport_la_SOURCES = pab_xxport.cpp pab_mapihd.cpp pab_pablib.cpp
#libkaddrbk_pab_xxport_la_LIBADD = $(XXLIBS)
#
#noinst_HEADERS = csvimportdialog.h
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)/kaddressbook
#service_DATA = csv_xxport.desktop vcard_xxport.desktop kde2_xxport.desktop \
#               bookmark_xxport.desktop eudora_xxport.desktop ldif_xxport.desktop \
#               gnokii_xxport.desktop opera_xxport.desktop pab_xxport.desktop
#
#rcdir = $(kde_datadir)/kaddressbook
#rc_DATA = csv_xxportui.rc vcard_xxportui.rc kde2_xxportui.rc \
#          bookmark_xxportui.rc eudora_xxportui.rc ldif_xxportui.rc \
#          opera_xxportui.rc pab_xxportui.rc $(gnokii_RCFILE)
