kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kmobile ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kmobile_skeleton_PART_SRCS skeleton.cpp )

kde4_automoc(${kmobile_skeleton_PART_SRCS})

kde4_add_plugin(kmobile_skeleton WITH_PREFIX ${kmobile_skeleton_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kmobile_skeleton )

target_link_libraries(kmobile_skeleton  ${QT_AND_KDECORE_LIBS} kmobiledevice )

install_targets(${LIB_INSTALL_DIR}/kde4 kmobile_skeleton )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES libkmobile_skeleton.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kmobile -I$(top_srcdir) $(all_includes) 
#
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = libkmobile_skeleton.la 
#
#kde_services_DATA = libkmobile_skeleton.desktop
#
#libkmobile_skeleton_la_SOURCES = skeleton.cpp
#libkmobile_skeleton_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) $(KDE_RPATH) -no-undefined -module
#libkmobile_skeleton_la_LIBADD = ../../libkmobiledevice.la
