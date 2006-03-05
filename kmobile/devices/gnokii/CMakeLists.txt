kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kmobile ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kmobile_gnokii_PART_SRCS gnokii_mobile.cpp gnokiiconfig.cpp )

kde4_automoc(${kmobile_gnokii_PART_SRCS})

kde4_add_ui_files(kmobile_gnokii_PART_SRCS gnokiiconfigui.ui )

kde4_add_plugin(kmobile_gnokii WITH_PREFIX ${kmobile_gnokii_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kmobile_gnokii )

target_link_libraries(kmobile_gnokii  ${QT_AND_KDECORE_LIBS} kmobiledevice kcal )

install_targets(${LIB_INSTALL_DIR}/kde4 kmobile_gnokii )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES libkmobile_gnokii.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kmobile -I$(top_srcdir) $(all_includes) 
#
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = libkmobile_gnokii.la 
#
#kde_services_DATA = libkmobile_gnokii.desktop
#
#libkmobile_gnokii_la_SOURCES = gnokii_mobile.cpp gnokiiconfigui.ui gnokiiconfig.cpp
#libkmobile_gnokii_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) $(KDE_RPATH) -no-undefined -module
#libkmobile_gnokii_la_LIBADD = ../../libkmobiledevice.la ../../../libkcal/libkcal.la $(XPMLIB) $(LIB_GNOKII)
