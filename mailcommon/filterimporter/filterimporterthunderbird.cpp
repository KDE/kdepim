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

#include "filterimporterthunderbird_p.h"
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
    } else if ( line.startsWith( QLatin1String( "type=" ) ) ) {
        line = cleanArgument(line, QLatin1String("type="));
        extractType(line, filter);
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
        const QStringList conditionsList = line.split( QLatin1String( "AND " ) );
        const int numberOfCond( conditionsList.count() );
        for ( int i = 0; i < numberOfCond; ++i )
        {
          splitConditions( conditionsList.at( i ), filter );
        }
    } else if( line.startsWith(QLatin1String("OR"))) {
        filter->pattern()->setOp(SearchPattern::OpOr);
        const QStringList conditionsList = line.split( QLatin1String( "OR " ) );
        const int numberOfCond( conditionsList.count() );
        for ( int i = 0; i < numberOfCond; ++i )
        {
          splitConditions( conditionsList.at( i ), filter );
        }
    } else {
      qDebug()<<" missing extract condition";
    }
}

void FilterImporterThunderbird::splitConditions( const QString&cond, MailCommon::MailFilter* filter )
{
  QString str = cond.trimmed();
  str.remove("(");
  str.remove(str.length()-1,1); //remove last )
  QStringList listOfCond = str.split( QLatin1Char( ',' ) );
  if ( listOfCond.count() < 3 ) {
    qDebug()<<"We have a pb in cond:"<<cond;
    return;
  }
  QString field = listOfCond.at( 0 );
  QString function = listOfCond.at( 1 );
  QString contents = listOfCond.at( 2 );
  if ( field == QLatin1String( "subject" ) ) {

  } else if ( field == QLatin1String( "date" ) ) {

  } else if ( field == QLatin1String( "from" ) ) {

  }

  if ( function == QLatin1String( "contains" ) ) {

  } else if ( function == QLatin1String( "isn't" ) ) {

  }

  if ( contents == QLatin1String( "" ) )
  {
    //TODO
  }
  qDebug()<<" field :"<<field<<" function :"<<function<<" contents :"<<contents<<" cond :"<<cond;

}

void FilterImporterThunderbird::extractActions(const QString& line, MailCommon::MailFilter* filter)
{
  QString actionName;
  if(line == QLatin1String("Move to folder")) {
    actionName = QLatin1String( "transfer" );
  } else if( line == QLatin1String("Forward")) {
    actionName = QLatin1String( "forward" );
  } else if( line == QLatin1String("Mark read")) {

  } else if( line == QLatin1String("Copy to folder")) {
    actionName = QLatin1String( "copy" );
  } else if( line == QLatin1String("AddTag")) {
    actionName = QLatin1String( "add tag" );
  } else if( line == QLatin1String("Delete")) {
    actionName = QLatin1String( "delete" );
  } else {
    qDebug()<<" missing convert method";
  }
}

void FilterImporterThunderbird::extractValues(const QString& line, MailCommon::MailFilter* filter)
{
}

void FilterImporterThunderbird::extractType(const QString& line, MailCommon::MailFilter* filter)
{
  //TODO
}

QString FilterImporterThunderbird::cleanArgument(const QString &line, const QString &removeStr)
{
    QString str = line;
    str.remove(removeStr);
    str.remove(QLatin1String("\""));
    str.remove(str.length()-1,1); //remove last "
    return str;
}

QList<MailFilter*> FilterImporterThunderbird::importFilter() const
{
  return mListMailFilter;
}
