/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Holger Hans Peter Freyther <freyther@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include "pluckerfilehandle.h"
#include "pluckerconfig.h"

#include <konnector.h>
#include <kstandarddirs.h>

#include <qfile.h>
#include <qtextstream.h>

namespace KSPlucker {

void PluckerFileHandle::addFile( const KURL& url, const QString& uid, bool site )
{
  QString md5 = KSync::Konnector::generateMD5Sum( url.path() );
  QString file = locateLocal( "appdata", "plucker-"+uid+"/"+md5+".jxl" );
  QString name = site ? "site" : "feed";

  QFile f( file );

  if ( f.exists() || !f.open( IO_WriteOnly) )
    return;

  QTextStream str( &f );
  str.setEncoding( QTextStream::UnicodeUTF8 );
  str << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  str << "<jxl lastEdited=\"2004-08-31T11:12:03\" "
    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
    "xsi:noNamespaceSchemaLocation=\"http://jpluck.sourceforge.net/jxl/jxl-2.1.xsd\">\n";
  str << "\t<"+name+">\n\t\t<name>KitchenSync Added URL"+md5+"</name>\n";
  str << "\t\t<uri>"+url.url()+"</uri>\n";
  str << "\t</"+name+">\n</jxl>\n";


  PluckerConfig* conf = PluckerConfig::self();
  QStringList lst = conf->pluckerFiles();
  if ( !lst.contains( file ) )
    lst.append( file );

  conf->setPluckerFiles( lst );
  conf->save( uid );
}

}
