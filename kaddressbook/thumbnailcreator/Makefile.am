## $Id$
#
# kioslave to generate thumbnails for vCard and LDIF-Files in Konqueror

INCLUDES = $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)
METASOURCES = AUTO

kde_module_LTLIBRARIES = ldifvcardthumbnail.la

ldifvcardthumbnail_la_SOURCES = ldifvcardcreator.cpp
ldifvcardthumbnail_la_LIBADD = $(LIB_KDECORE) $(LIB_KABC)
ldifvcardthumbnail_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) -lkdefx

noinst_HEADERS = ldifvcardcreator.h

services_DATA = ldifvcardthumbnail.desktop
servicesdir = $(kde_servicesdir)

