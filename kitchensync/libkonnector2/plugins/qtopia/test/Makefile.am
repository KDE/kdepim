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

INCLUDES= -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/kitchensync/libksync $(all_includes)

check_PROGRAMS = kdetest todometa contactmeta datebookmeta maintest

kdetest_LDADD =  $(LIB_KDEUI) $(LIB_QT)
kdetest_SOURCES =  kdetest.cpp
kdetest_LDFLAGS = $(all_libraries) $(KDE_RPATH)

todometa_LDADD = $(LIB_KDEUI) $(LIB_QT)
todometa_SOURCES = todometa.cpp
todometa_LDFLAGS = $(all_libraries) $(KDE_RPATH)

contactmeta_LDADD = $(top_builddir)/kitchensync/lib/libksync.la \
	$(top_builddir)/kitchensync/libkonnector2/plugins/qtopia/libqtopiakonnector.la \
	$(top_builddir)/kitchensync/libkonnector2/libkonnector.la
contactmeta_SOURCES = contactmeta.cpp
contactmeta_LDFLAGS = $(all_libraries) $(KDE_RPATH)

datebookmeta_LDADD = $(LIB_KDEUI) $(LIB_QT)
datebookmeta_SOURCES = datebookmeta.cpp
datebookmeta_LDFLAGS = $(all_libraries) $(KDE_RPATH)

maintest_LDADD = $(LIB_KDEUI) $(LIB_QT) $(top_builddir)/kitchensync/lib/libksync.la \
	$(top_builddir)/kitchensync/libkonnector2/plugins/qtopia/libqtopiakonnector.la \
	$(top_builddir)/kitchensync/libkonnector2/libkonnector.la
maintest_SOURCES = main.cpp
maintest_LDFLAGS = $(all_libraries) $(KDE_RPATH)

METASOURCES = AUTO

