/*
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2011 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filterimporterthunderbird.h"
#include <QTextStream>
#include <QDebug>
#include "mailfilter.h"

using namespace MailCommon;

FilterImporterThunderbird::FilterImporterThunderbird( QFile *file )
{
  QTextStream stream(file);
  MailFilter *filter = 0;
  while (!stream.atEnd()) {
    QString line = stream.readLine();
    qDebug()<<" line :"<<line;
    if ( line.startsWith( QLatin1String( "name=" ) ) ) {
      if ( filter )
        mListMailFilter.append( filter );
      filter = new MailFilter();
    } else if ( line.startsWith( QLatin1String( "action=" ) ) ) {

    } else if ( line.startsWith( QLatin1String( "enabled=" ) ) ) {

    } else if ( line.startsWith( QLatin1String( "condition=" ) ) ) {

    } else if ( line.startsWith( QLatin1String( "actionValue=" ) ) ) {

    }
  }
  if ( filter )
    mListMailFilter.append( filter );
}

FilterImporterThunderbird::~FilterImporterThunderbird()
{
}

QList<MailFilter*> FilterImporterThunderbird::importFilter() const
{
  //TODO
  return QList<MailFilter*>();
}
