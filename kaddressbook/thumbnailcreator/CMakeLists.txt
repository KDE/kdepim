



########### next target ###############

set(ldifvcardthumbnail_PART_SRCS ldifvcardcreator.cpp )

kde4_automoc(${ldifvcardthumbnail_PART_SRCS})

kde4_add_plugin(ldifvcardthumbnail ${ldifvcardthumbnail_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} ldifvcardthumbnail )

target_link_libraries(ldifvcardthumbnail  ${KDE4_KDECORE_LIBS} ${KDE4_KDEFX_LIBS} kabc)

install(TARGETS ldifvcardthumbnail  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES ldifvcardthumbnail.desktop  DESTINATION ${SERVICES_INSTALL_DIR})




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
