#    This file is part of the KDE libraries
#    Copyright (C) 2003 Mathias Froehlich <MAthias.Froehlich@web.de>

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

lib_LTLIBRARIES = libthreadedkonnector.la

INCLUDES = -I$(top_srcdir)/ -I$(top_srcdir)/kitchensync/libksync -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/libkcal $(all_includes)

libthreadedkonnector_la_INCLUDES = -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/kitchensync/lib -I../../  $(all_includes)

libthreadedkonnector_la_LIBADD = ../../libkonnector.la
libthreadedkonnector_la_LDFLAGS = -module $(KDE_PLUGIN)
libthreadedkonnector_la_SOURCES = threadedkonnector.cpp konnectorconfig.cpp \
  clientmanager.cpp clientthread.cpp
threadedkonnectordatadir = $(kde_servicesdir)/kresources/konnector
threadedkonnectordata_DATA = threadedkonnector.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/konnector_threaded.pot
