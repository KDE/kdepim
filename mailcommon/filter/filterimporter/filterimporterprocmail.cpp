/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filterimporterprocmail_p.h"

#include "filtermanager.h"
#include "mailfilter.h"

#include <KDebug>

#include <QFile>

using namespace MailCommon;

FilterImporterProcmail::FilterImporterProcmail( QFile *file )
  :FilterImporterAbstract()
{
  QTextStream stream(file);
  MailFilter *filter = 0;
  while ( !stream.atEnd() ) {
    QString line = stream.readLine();
    kDebug() << " line :" << line << " filter " << filter;
    filter = parseLine( stream, line, filter );
  }

  if ( filter ) {
    appendFilter(filter);
  }
}

FilterImporterProcmail::~FilterImporterProcmail()
{
}


MailCommon::MailFilter *FilterImporterProcmail::parseLine( QTextStream &stream,
                                                           QString line,
                                                           MailCommon::MailFilter *filter )
{
  if ( line.isEmpty() ) {
    //Empty line
    return filter;
  } else if ( line.startsWith( QLatin1Char( '#' ) ) ){
    //Commented line
    return filter;
  } else if ( line.startsWith( QLatin1String( ":0" ) ) ) {
    if ( filter ) {
      appendFilter(filter);
    }
    filter = new MailFilter();

  } else if ( line.startsWith( QLatin1Char( '*' ) ) ) {
    //Condition
  } else if ( line.startsWith( QLatin1Char( '!' ) ) ) {
    //Redirect email
  } else if ( line.startsWith( QLatin1Char( '|' ) ) ) {
    //Shell
  } else if ( line.startsWith( QLatin1Char( '{' ) ) ) {
    //Block
  } else if ( line.startsWith( QLatin1Char( '}' ) ) ) {
    //End block
  } else {
    //Folder
  }


  return filter;
}
