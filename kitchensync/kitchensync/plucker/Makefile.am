#
#    This file is part of KitchenSync.
#
#    Copyright (c) 2004 Holger Hans Peter Freyther <freyther@kde.org>
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


konqservicedir = $(kde_datadir)/konqueror/servicemenus
konqservice_DATA = kitchensync-pluck.desktop kitchensync-pluck-rdf.desktop

servicedir = $(kde_servicesdir)/kitchensync
service_DATA = plucker.desktop


INCLUDES = -I$(top_srcdir)/kitchensync/kitchensync/lib \
  -I$(top_srcdir)/kitchensync/libksync \
  -I$(top_srcdir)/kitchensync/libkonnector2 \
  -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libksync_pluckerpart.la
libksync_pluckerpart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libksync_pluckerpart_la_LIBADD  = ../lib/libkitchensyncui.la
libksync_pluckerpart_la_SOURCES = pluckerpart.cpp pluckerinterface.skel \
                                  kspluckerconfigwidget.ui pluckerconfig.cpp  \
                                  pluckerfilehandle.cpp pluckerprocesshandler.cpp

METASOURCES = AUTO

kspluckerconfigwidget.lo: kspluckerconfigwidget.ui.h

