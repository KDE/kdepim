kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kresources/opengroupware/soap ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kio_opengroupware_PART_SRCS opengroupware.cpp webdavhandler.cpp )

kde4_automoc(${kio_opengroupware_PART_SRCS})

kde4_add_plugin(kio_opengroupware ${kio_opengroupware_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kio_opengroupware )

target_link_libraries(kio_opengroupware  ${QT_AND_KDECORE_LIBS} kcal kdepim kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kio_opengroupware )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES opengroupware.protocol opengroupwares.protocol )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kresources/opengroupware/soap \
#  $(all_includes)
#
#noinst_HEADERS = opengroupware.h
#
#METASOURCES = AUTO
#
#kdelnkdir = $(kde_servicesdir)
#kdelnk_DATA = opengroupware.protocol opengroupwares.protocol
#
#kde_module_LTLIBRARIES =  kio_opengroupware.la
#
#kio_opengroupware_la_SOURCES = opengroupware.cpp webdavhandler.cpp
#kio_opengroupware_la_LIBADD = $(top_builddir)/libkcal/libkcal.la \
#   $(top_builddir)/libkdepim/libkdepim.la $(LIB_KIO)
#kio_opengroupware_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#
