#    This file is part of the KDE libraries
#    Copyright (C) 2002 Maurus Erni <erni@pocketviewer.de>

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.

SUBDIRS = libpv

lib_LTLIBRARIES = libpvkonnector.la

INCLUDES = -I$(top_srcdir)/ -I$(top_srcdir)/kitchensync/lib -I$(top_srcdir)/kitchensync/libkonnector -I$(top_srcdir)/libkcal -Ilibpv $(all_includes)

libpvkonnector_la_INCLUDES = -I$(top_srcdir)/kitchensync/libkonnector -I$(top_srcdir)/kitchensync/lib -I../../   $(all_includes)
libpvkonnector_la_LIBADD = ../../libkonnector.la ../../../libksync.la
# libpvkonnector_la_LIBADD = libpv/libpv.la
libpvkonnector_la_LDFLAGS = $(all_libraries)  $(KDE_RPATH) -module 
libpvkonnector_la_SOURCES = pvkonnector.cpp casiopvlink.cpp addressbook.cpp event.cpp todo.cpp casiopvlinkiface.skel

pvdatadir = $(kde_datadir)/kitchensync
pvdata_DATA = pv.desktop

METASOURCES = AUTO

