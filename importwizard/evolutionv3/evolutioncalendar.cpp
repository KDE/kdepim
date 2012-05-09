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

#include "evolutioncalendar.h"
#include "evolutionutil.h"

#include <KDebug>

#include <QFile>
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>

EvolutionCalendar::EvolutionCalendar(const QString& filename,ImportWizard *parent)
  :AbstractCalendar(parent)
{
  //Read gconf file
  QFile file(filename);
  if ( !file.open( QIODevice::ReadOnly ) ) {
    qDebug()<<" We can't open file"<<filename;
    return;
  }
  QDomDocument doc;
  if ( !EvolutionUtil::loadInDomDocument( &file, doc ) )
    return;
  QDomElement config = doc.documentElement();

  if ( config.isNull() ) {
    kDebug() << "No config found";
    return;
  }
  for ( QDomElement e = config.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
    const QString tag = e.tagName();
    if ( tag == QLatin1String( "entry" ) ) {
      if ( e.hasAttribute( "name" ) ) {
        const QString attr = e.attribute("name");
        if ( attr == QLatin1String( "sources" ) ) {
          readCalendar(e);
        } else {
        qDebug()<<" attr unknown "<<attr;
        }
      }
    }
  }
}

EvolutionCalendar::~EvolutionCalendar()
{

}

void EvolutionCalendar::readCalendar(const QDomElement &calendar)
{
  for ( QDomElement calendarConfig = calendar.firstChildElement(); !calendarConfig.isNull(); calendarConfig = calendarConfig.nextSiblingElement() ) {
    if(calendarConfig.tagName() == QLatin1String("li")) {
      const QDomElement stringValue = calendarConfig.firstChildElement();
      extractCalendarInfo(stringValue.text());
    }
  }
}

void EvolutionCalendar::extractCalendarInfo(const QString& info)
{
  qDebug()<<" info "<<info;
  //Read QDomElement
  QDomDocument cal;
  if ( !EvolutionUtil::loadInDomDocument( info, cal ) )
    return;
  QDomElement domElement = cal.documentElement();

  if ( domElement.isNull() ) {
    kDebug() << "Account not found";
    return;
  }
  for ( QDomElement e = domElement.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
    const QString tag = e.tagName();
    qDebug()<<" EvolutionCalendar::extractCalendarInfo tag :"<<tag;
  }
}
