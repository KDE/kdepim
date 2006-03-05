kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kaddressbook/common ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcal_kabc_PART_SRCS resourcekabc.cpp resourcekabcconfig.cpp )

kde4_automoc(${kcal_kabc_PART_SRCS})

kde4_add_plugin(kcal_kabc ${kcal_kabc_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcal_kabc )

target_link_libraries(kcal_kabc  ${QT_AND_KDECORE_LIBS} kcal kabcommon )

install_targets(${LIB_INSTALL_DIR}/kde4 kcal_kabc )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kresources/kcal FILES kabc.desktop )
install_files( /include/libkcal FILES resourcekabc.h resourcekabcconfig.h )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) -I$(top_builddir) \
#           -I$(top_builddir)/kaddressbook/common \
#           -I$(top_srcdir)/kaddressbook/common \
#             $(all_includes)
#
#kde_module_LTLIBRARIES = kcal_kabc.la
#
#kcal_kabc_la_SOURCES = resourcekabc.cpp resourcekabcconfig.cpp
#kcal_kabc_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kcal_kabc_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la \
#                       $(top_builddir)/kaddressbook/common/libkabcommon.la
#
#libkcalincludedir = $(includedir)/libkcal
#libkcalinclude_HEADERS = resourcekabc.h resourcekabcconfig.h
#
#servicedir = $(kde_servicesdir)/kresources/kcal
#service_DATA = kabc.desktop
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kres_birthday.pot
