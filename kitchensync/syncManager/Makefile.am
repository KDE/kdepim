## Makefile.am                  KitchenSync
##
## Copyright (C) 2001 by Adriaan de Groot
##
## This program is part of KitchenSync, the KDE handheld-device
## synchronisation framework.
##
 
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program in a file called COPYING; if not, write to
## the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
## MA 02139, USA.
##
 
##
## Bug reports and questions can be sent to adridg@cs.kun.nl
##

bin_PROGRAMS = kitchensync
noinst_LIBRARIES = libsyncmanager.a

METASOURCES = AUTO
 
INCLUDES = $(QT_INCLUDES) $(KDE_INCLUDES) -I$(top_srcdir) -DDEBUG
LDFLAGS = $(KDE_LDFLAGS) $(LIB_KFILE) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_QT)

kitchensync_SOURCES = main.cc systray.cc logWindow.cc syncManager.cc \
	configdialog.cc \
	syncManagerIface.skel
kitchensync_LDADD = ../lib/libkitchensync.a

libsyncmanager_a_SOURCES = syncManagerIface.stub


kitchensyncicondir = $(kde_datadir)/kitchensync/icons
kitchensyncicon_ICON = busysync hotsync

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = syncDaemon.desktop

KDE_ICON = kitchensync

## $Log:$
