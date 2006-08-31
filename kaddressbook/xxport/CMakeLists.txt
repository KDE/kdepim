
include_directories( ${CMAKE_SOURCE_DIR}/kaddressbook/interfaces ${CMAKE_SOURCE_DIR}/kaddressbook  )

configure_file (config-xxport.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-xxport.h )

########### next target ###############

set(kaddrbk_csv_xxport_PART_SRCS csv_xxport.cpp csvimportdialog.cpp dateparser.cpp )

kde4_automoc(${kaddrbk_csv_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_csv_xxport WITH_PREFIX ${kaddrbk_csv_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_csv_xxport)

target_link_libraries(kaddrbk_csv_xxport  ${KDE4_KDECORE_LIBS} kaddressbook ${KDE4_KABC_LIBS})

install(TARGETS kaddrbk_csv_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_vcard_xxport_PART_SRCS vcard_xxport.cpp )

kde4_automoc(${kaddrbk_vcard_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_vcard_xxport WITH_PREFIX ${kaddrbk_vcard_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_vcard_xxport )

target_link_libraries(kaddrbk_vcard_xxport  ${KDE4_KDECORE_LIBS} kaddressbook qgpgme )

install(TARGETS kaddrbk_vcard_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_kde2_xxport_PART_SRCS kde2_xxport.cpp )

kde4_automoc(${kaddrbk_kde2_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_kde2_xxport WITH_PREFIX ${kaddrbk_kde2_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_kde2_xxport )

target_link_libraries(kaddrbk_kde2_xxport  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} kaddressbook)

install(TARGETS kaddrbk_kde2_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_bookmark_xxport_PART_SRCS bookmark_xxport.cpp )

kde4_automoc(${kaddrbk_bookmark_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_bookmark_xxport WITH_PREFIX ${kaddrbk_bookmark_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_bookmark_xxport )

target_link_libraries(kaddrbk_bookmark_xxport ${KDE4_KDECORE_LIBS} kaddressbook )

install(TARGETS kaddrbk_bookmark_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_eudora_xxport_PART_SRCS eudora_xxport.cpp )

kde4_automoc(${kaddrbk_eudora_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_eudora_xxport WITH_PREFIX ${kaddrbk_eudora_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_eudora_xxport )

target_link_libraries(kaddrbk_eudora_xxport  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} kaddressbook)

install(TARGETS kaddrbk_eudora_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_ldif_xxport_PART_SRCS ldif_xxport.cpp )

kde4_automoc(${kaddrbk_ldif_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_ldif_xxport WITH_PREFIX ${kaddrbk_ldif_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_ldif_xxport )

target_link_libraries(kaddrbk_ldif_xxport  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} kaddressbook )

install(TARGETS kaddrbk_ldif_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_opera_xxport_PART_SRCS opera_xxport.cpp )

kde4_automoc(${kaddrbk_opera_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_opera_xxport WITH_PREFIX ${kaddrbk_opera_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_opera_xxport )

target_link_libraries(kaddrbk_opera_xxport  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} kaddressbook)

install(TARGETS kaddrbk_opera_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_pab_xxport_PART_SRCS pab_xxport.cpp pab_mapihd.cpp pab_pablib.cpp )

kde4_automoc(${kaddrbk_pab_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_pab_xxport WITH_PREFIX ${kaddrbk_pab_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_pab_xxport )

target_link_libraries(kaddrbk_pab_xxport  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} kaddressbook)

install(TARGETS kaddrbk_pab_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_gmx_xxport_PART_SRCS gmx_xxport.cpp )

kde4_automoc(${kaddrbk_gmx_xxport_PART_SRCS})

kde4_add_plugin(kaddrbk_gmx_xxport WITH_PREFIX ${kaddrbk_gmx_xxport_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_gmx_xxport )

target_link_libraries(kaddrbk_gmx_xxport  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} kaddressbook )

install(TARGETS kaddrbk_gmx_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

if(GNOKII_FOUND)
   set(kaddrbk_gnokii_xxport_PART_SRCS gnokii_xxport.cpp )

   kde4_automoc(${kaddrbk_gnokii_xxport_PART_SRCS})

   kde4_add_plugin(kaddrbk_gnokii_xxport WITH_PREFIX ${kaddrbk_gnokii_xxport_PART_SRCS})

   kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kaddrbk_gnokii_xxport )

   target_link_libraries(kaddrbk_gnokii_xxport  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} ${GNOKII_LIBRARIES} kaddressbook )

   install(TARGETS kaddrbk_gnokii_xxport  DESTINATION ${PLUGIN_INSTALL_DIR})
   install(FILES gnokii_xxport.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kaddressbook)
   install(FILES gnokii_xxportui.rc DESTINATION ${DATA_INSTALL_DIR}/kaddressbook)
#libkaddrbk_gnokii_xxport_la_SOURCES = gnokii_xxport.cpp
#libkaddrbk_gnokii_xxport_la_LIBADD = $(XPMLIB) $(LIB_GNOKII) $(XXLIBS)
endif(GNOKII_FOUND)

install( FILES csv_xxport.desktop vcard_xxport.desktop kde2_xxport.desktop bookmark_xxport.desktop eudora_xxport.desktop ldif_xxport.desktop opera_xxport.desktop pab_xxport.desktop gmx_xxport.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kaddressbook)

install( FILES csv_xxportui.rc vcard_xxportui.rc kde2_xxportui.rc            bookmark_xxportui.rc eudora_xxportui.rc ldif_xxportui.rc opera_xxportui.rc pab_xxportui.rc gmx_xxportui.rc DESTINATION ${DATA_INSTALL_DIR}/kaddressbook)




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
