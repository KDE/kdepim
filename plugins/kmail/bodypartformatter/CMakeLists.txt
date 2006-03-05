kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kmail ${CMAKE_SOURCE_DIR}/libkdepim ${CMAKE_SOURCE_DIR}/libkmime ${CMAKE_SOURCE_DIR}/mimelib ${CMAKE_SOURCE_DIR}/libemailfunctions ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kmail_bodypartformatter_text_vcard_PART_SRCS text_vcard.cpp )

kde4_automoc(${kmail_bodypartformatter_text_vcard_PART_SRCS})

kde4_add_plugin(kmail_bodypartformatter_text_vcard WITH_PREFIX ${kmail_bodypartformatter_text_vcard_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kmail_bodypartformatter_text_vcard )

target_link_libraries(kmail_bodypartformatter_text_vcard  ${QT_AND_KDECORE_LIBS} kdepim kmime emailfunctions )

install_targets(${LIB_INSTALL_DIR}/kde4 kmail_bodypartformatter_text_vcard )


########### next target ###############

set(kmail_bodypartformatter_text_calendar_PART_SRCS text_calendar.cpp )

kde4_automoc(${kmail_bodypartformatter_text_calendar_PART_SRCS})

kde4_add_plugin(kmail_bodypartformatter_text_calendar WITH_PREFIX ${kmail_bodypartformatter_text_calendar_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kmail_bodypartformatter_text_calendar )

target_link_libraries(kmail_bodypartformatter_text_calendar  ${QT_AND_KDECORE_LIBS} kcal kmailprivate )

install_targets(${LIB_INSTALL_DIR}/kde4 kmail_bodypartformatter_text_calendar )


########### next target ###############

set(kmail_bodypartformatter_text_xdiff_PART_SRCS text_xdiff.cpp )

kde4_automoc(${kmail_bodypartformatter_text_xdiff_PART_SRCS})

kde4_add_plugin(kmail_bodypartformatter_text_xdiff WITH_PREFIX ${kmail_bodypartformatter_text_xdiff_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kmail_bodypartformatter_text_xdiff )

target_link_libraries(kmail_bodypartformatter_text_xdiff  ${QT_AND_KDECORE_LIBS} ${QT_QT3SUPPORT_LIBRARY} )

install_targets(${LIB_INSTALL_DIR}/kde4 kmail_bodypartformatter_text_xdiff )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kmail/plugins/bodypartformatter FILES text_vcard.desktop text_calendar.desktop text_xdiff.desktop )

kde4_footer()



#original Makefile.am contents follow:

#KDE_OPTIONS = nofinal
#KDE_CXXFLAGS = $(USE_RTTI)
#
#INCLUDES = -I$(top_srcdir)/kmail \
#           -I$(top_srcdir)/libkdepim \
#           -I$(top_srcdir)/libkmime \
#           -I$(top_srcdir)/mimelib \
#           -I$(top_srcdir)/libemailfunctions \
#           -I$(top_srcdir) \
#           -I$(top_builddir)/libkdepim \
#           $(all_includes)
#
#kde_module_LTLIBRARIES = libkmail_bodypartformatter_text_vcard.la \
#                         libkmail_bodypartformatter_text_calendar.la \
#                         libkmail_bodypartformatter_text_xdiff.la
#
#libkmail_bodypartformatter_text_vcard_la_SOURCES = text_vcard.cpp
#libkmail_bodypartformatter_text_vcard_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) -no-undefined
#libkmail_bodypartformatter_text_vcard_la_LIBADD  = $(LIB_QT) $(LIB_KABC) \
#                                                   ../../../libkdepim/libkdepim.la \
#                                                   ../../../libkmime/libkmime.la \
#                                                   ../../../libemailfunctions/libemailfunctions.la
#
#libkmail_bodypartformatter_text_calendar_la_SOURCES = text_calendar.cpp
#libkmail_bodypartformatter_text_calendar_la_LDFLAGS = \
#  $(all_libraries) -module $(KDE_PLUGIN) -no-undefined
#libkmail_bodypartformatter_text_calendar_la_LIBADD  = $(LIB_QT) $(LIB_KABC) \
#  $(top_builddir)/libkcal/libkcal.la $(top_builddir)/kmail/libkmailprivate.la
#
#libkmail_bodypartformatter_text_xdiff_la_SOURCES = text_xdiff.cpp
#libkmail_bodypartformatter_text_xdiff_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#libkmail_bodypartformatter_text_xdiff_la_LIBADD  = $(LIB_QT) $(LIB_QT3SUPPORT)
#
#
#linkdir = $(kde_datadir)/kmail/plugins/bodypartformatter
#link_DATA = text_vcard.desktop text_calendar.desktop text_xdiff.desktop
#
#METASOURCES = AUTO
#
#messages:
#	$(XGETTEXT) text_vcard.cpp -o $(podir)/kmail_text_vcard_plugin.pot
#	$(XGETTEXT) text_calendar.cpp -o $(podir)/kmail_text_calendar_plugin.pot
#	$(XGETTEXT) text_xdiff.cpp -o $(podir)/kmail_text_xdiff_plugin.pot
