#    This file is part of the KDE libraries
#    Copyright (C) 2002 Holger Freyther <freyther@kde.org>

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

#SUBDIRS = tests

lib_LTLIBRARIES = libqtopiakonnector.la

INCLUDES = -I$(top_srcdir)/ -I$(top_srcdir)/kitchensync/lib -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/libkcal $(all_includes)

libqtopiakonnector_la_INCLUDES = -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/kitchensync/lib -I../../   $(all_includes)
libqtopiakonnector_la_LIBDADD = ../../libkonnector.la ../../../libksync.la
libqtopiakonnector_la_LDFLAGS = $(all_libraries)  $(KDE_RPATH) -module
libqtopiakonnector_la_SOURCES = konnector.cpp socket.cpp addressbook.cpp \
				categoryedit.cpp datebook.cpp helper.cpp \
				opiecategories.cpp todo.cpp desktop.cpp \
				md5map.cpp metaaddressbook.cpp \
				metadatebook.cpp metatodo.cpp \
				qtopiaconfig.cpp device.cpp

qtopiadatadir = $(kde_datadir)/kitchensync
qtopiadata_DATA = qtopia.desktop

qtopiapicsdir = $(kde_datadir)/kitchensync/pics
qtopiapics_DATA = opie.jpg

SUBDIRS = . test

METASOURCES = AUTO

