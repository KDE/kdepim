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

INCLUDES= -I$(top_srcdir)/kitchensync/libkonnector \
	-I$(top_srcdir)/kitchensync/lib  \
	-I$(top_srcdir)/kitchensync/libkonnector/plugins \
	-I$(top_srcdir)/libkcal \
	$(all_includes)
LDADD = $(top_builddir)/kitchensync/libkonnector/libkonnector.la $(top_builddir)/kitchensync/libkonnector/plugins/opie/libopiekonnector.la
LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined

check_PROGRAMS = opieimport todoreader eventreader

#opieimport_LDADD = ../../../libkonnector.la ../libopiekonnector.la
opieimport_SOURCES =  convertermain.cc

todoreader_LDADD = $(top_builddir)/libkcal/libkcal.la \
	$(top_builddir)/kitchensync/lib/libksync.la \
	$(top_builddir)/kitchensync/libkonnector/plugins/opie/libopiekonnector.la \
	$(top_builddir)/kitchensync/libkonnector/libkonnector.la
todoreader_SOURCES = todoreader.cpp

eventreader_LDADD = $(top_builddir)/libkcal/libkcal.la \
	$(top_builddir)/kitchensync/lib/libksync.la \
	$(top_builddir)/kitchensync/libkonnector/plugins/opie/libopiekonnector.la \
	$(top_builddir)/kitchensync/libkonnector/libkonnector.la
eventreader_SOURCES = eventreader.cpp

METASOURCES = AUTO

