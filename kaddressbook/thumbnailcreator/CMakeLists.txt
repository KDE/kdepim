kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(ldifvcardthumbnail_PART_SRCS ldifvcardcreator.cpp )

kde4_automoc(${ldifvcardthumbnail_PART_SRCS})

kde4_add_plugin(ldifvcardthumbnail ${ldifvcardthumbnail_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} ldifvcardthumbnail )

target_link_libraries(ldifvcardthumbnail  ${KDE4_KDECORE_LIBS} kdefx kabc)

install_targets(${PLUGIN_INSTALL_DIR} ldifvcardthumbnail )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES ldifvcardthumbnail.desktop )

kde4_footer()



#original Makefile.am contents follow:

## kioslave to generate thumbnails for vCard and LDIF-Files in Konqueror
#INCLUDES = $(all_includes)
#
#kde_module_LTLIBRARIES = ldifvcardthumbnail.la
#
#ldifvcardthumbnail_la_SOURCES = ldifvcardcreator.cpp
#ldifvcardthumbnail_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
#ldifvcardthumbnail_la_LIBADD = $(LIB_KDECORE) $(LIB_KABC) -lkdefx
#
#noinst_HEADERS = ldifvcardcreator.h
#
#METASOURCES = AUTO
#
#servicesdir = $(kde_servicesdir)
#services_DATA = ldifvcardthumbnail.desktop
