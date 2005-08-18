# kioslave to generate thumbnails for vCard and LDIF-Files in Konqueror
INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = ldifvcardthumbnail.la

ldifvcardthumbnail_la_SOURCES = ldifvcardcreator.cpp
ldifvcardthumbnail_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
ldifvcardthumbnail_la_LIBADD = $(LIB_KDECORE) $(LIB_KABC) -lkdefx

noinst_HEADERS = ldifvcardcreator.h

METASOURCES = AUTO

servicesdir = $(kde_servicesdir)
services_DATA = ldifvcardthumbnail.desktop
