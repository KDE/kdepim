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

SUBDIRS = docs

bin_PROGRAMS = pvdaemon

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

include_HEADERS = pvdaemoniface.h
noinst_HEADERS = pvdaemon.h
INCLUDES = $(all_includes)
pvdaemon_SOURCES = pvdaemon.cpp main.cpp pvmail.cpp pvcontactuntitled.cpp serialexception.cpp serial.cpp pvtodo.cpp pvschedule.cpp pvreminder.cpp pvquickmemo.cpp pvpocketsheet.cpp pvmultidate.cpp pvmemo.cpp pvexpense.cpp pvdataentryexception.cpp pvcontact.cpp protocolexception.cpp protocol.cpp casiopvexception.cpp casiopv.cpp baseexception.cpp utils.cpp pvdaemoniface.skel
pvdaemon_LDADD    = $(LIB_KIO)
pvdaemon_LDFLAGS = $(all_libraries)  $(KDE_RPATH)

METASOURCES = AUTO

