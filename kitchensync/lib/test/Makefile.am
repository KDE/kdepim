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

INCLUDES=  -I$(top_srcdir)/kitchensync/lib -I$(top_srcdir)/libkcal $(all_includes)
check_PROGRAMS = todotest eventtest abtest

todotest_LDADD = ../libksync.la $(top_builddir)/libkcal/libkcal.la $(LIB_KDEUI) $(LIB_QT)
todotest_SOURCES =  todomain.cpp
todotest_LDFLAGS = $(all_libraries) $(KDE_RPATH)

eventtest_LDADD = ../libksync.la $(top_builddir)/libkcal/libkcal.la $(LIB_KDEUI) $(LIB_QT)
eventtest_SOURCES =  eventmain.cpp
eventtest_LDFLAGS = $(all_libraries) $(KDE_RPATH)

abtest_LDADD = ../libksync.la -lkabc $(LIB_KDEUI) $(LIB_QT)
abtest_SOURCES =  abmain.cpp
abtest_LDFLAGS = $(all_libraries) $(KDE_RPATH)

METASOURCES = AUTO

