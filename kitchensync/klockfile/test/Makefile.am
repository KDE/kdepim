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

INCLUDES= -I$(top_srcdir)/kitchensync/klockfile $(all_includes)

check_PROGRAMS = ksharedtest kshareddevicetest

ksharedtest_LDADD = ../libksharedfile.la $(LIB_KDEUI) $(LIB_QT)
ksharedtest_LDFLAGS = $(all_libraries)
ksharedtest_SOURCES = main.cpp ksharedtest.ui

kshareddevicetest_LDADD = ../libksharedfile.la $(LIB_KDEUI) $(LIB_QT)
kshareddevicetest_SOURCES =  kshareddevicetest.cpp
kshareddevicetest_LDFLAGS = $(all_libraries)

METASOURCES = AUTO

