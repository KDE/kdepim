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
      line = cleanArgument(line, QLatin1String("name="));
      filter->setToolbarName(line);
    } else if ( line.startsWith( QLatin1String( "action=" ) ) ) {
        line = cleanArgument(line, QLatin1String("action="));
        extractActions(line, filter);
        //TODO look at value here.
    } else if ( line.startsWith( QLatin1String( "enabled=" ) ) ) {
        line = cleanArgument(line, QLatin1String("enabled="));
        if(line == QLatin1String("no"))
            filter->setEnabled(false);
    } else if ( line.startsWith( QLatin1String( "condition=" ) ) ) {
        line = cleanArgument(line, QLatin1String("condition="));
        extractConditions(line, filter);
    } else if ( line.startsWith( QLatin1String( "actionValue=" ) ) ) {
        line = cleanArgument(line, QLatin1String("actionValue="));
        extractValues(line, filter);
    }
  }
  if ( filter )
    mListMailFilter.append( filter );
}

FilterImporterThunderbird::~FilterImporterThunderbird()
{
}

void FilterImporterThunderbird::extractConditions(const QString& line, MailCommon::MailFilter* filter)
{
    if(line.startsWith(QLatin1String("AND"))) {
        filter->pattern()->setOp(SearchPattern::OpAnd);
    } else if( line.startsWith(QLatin1String("OR"))) {
        filter->pattern()->setOp(SearchPattern::OpOr);
    }
}

void FilterImporterThunderbird::extractActions(const QString& line, MailCommon::MailFilter* filter)
{
    if(line == QLatin1String("Move to folder")) {

    } else if( line == QLatin1String("Forward")) {

    } else if( line == QLatin1String("Mark read")) {

    } else if( line == QLatin1String("Copy to folder")) {

    } else if( line == QLatin1String("AddTag")) {

    } else if( line == QLatin1String("Delete")) {

    } else {
        qDebug()<<" missing convert method";
    }
}

void FilterImporterThunderbird::extractValues(const QString& line, MailCommon::MailFilter* filter)
{
}


QString FilterImporterThunderbird::cleanArgument(const QString &line, const QString &removeStr)
{
    QString str = line;
    str.remove(removeStr);
    str.remove("\"");
    str.remove(removeStr.length()-1,1); //remove last "
    return str;
}

QList<MailFilter*> FilterImporterThunderbird::importFilter() const
{
  return mListMailFilter;
}
