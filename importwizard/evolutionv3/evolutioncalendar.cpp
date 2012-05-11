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
#include <QDir>
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>

EvolutionCalendar::EvolutionCalendar(const QString& filename,ImportWizard *parent)
  :AbstractCalendar(parent)
{
  //Read gconf file
  QFile file(filename);
  if ( !file.open( QIODevice::ReadOnly ) ) {
    kDebug()<<" We can't open file"<<filename;
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
  mCalendarPath = QDir::homePath() + QLatin1String("/.local/share/evolution/calendar/");
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
  QString base_uri;
  if(domElement.hasAttribute(QLatin1String("base_uri"))) {
    base_uri = domElement.attribute(QLatin1String("base_uri"));
  }
  if(base_uri == QLatin1String("local:")) {
    //TODO <group uid="1326983249.24740.3@kspread" name="Sur cet ordinateur" base_uri="local:" readonly="no">
    for ( QDomElement e = domElement.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
      const QString tag = e.tagName();
      if(tag == QLatin1String("source")) {
        QString name;
        QMap<QString, QVariant> settings;
        if(e.hasAttribute(QLatin1String("uid"))) {
        }
        if(e.hasAttribute(QLatin1String("name"))) {
          name = e.attribute(QLatin1String("name"));
          settings.insert(QLatin1String("DisplayName"), name);
        }
        if(e.hasAttribute(QLatin1String("relative_uri"))) {
          const QString path = mCalendarPath + e.attribute(QLatin1String("relative_uri")) + QLatin1String("/calendar.ics");
          settings.insert(QLatin1String("Path"), path);
        }
        if(e.hasAttribute(QLatin1String("color_spec"))) {
          const QString color = e.attribute(QLatin1String("color_spec"));
          //Need id.
          //TODO
        }
        QDomElement propertiesElement = e.firstChildElement();
        if(!propertiesElement.isNull()) {
          for ( QDomElement property = propertiesElement.firstChildElement(); !property.isNull(); property = property.nextSiblingElement() ) {
            const QString propertyTag = property.tagName();
            if(propertyTag == QLatin1String("property")) {
              if(property.hasAttribute(QLatin1String("name"))) {
                const QString propertyName = property.attribute(QLatin1String("name"));
                if(propertyName == QLatin1String("custom-file-readonly")) {
                  if(property.hasAttribute(QLatin1String("value"))) {
                    if(property.attribute(QLatin1String("value")) == QLatin1String("1")) {
                      settings.insert(QLatin1String("ReadOnly"), true);
                    }
                  }
                }
              }
            } else {
              qDebug()<<" property unknown :"<<propertyTag;
            }
          }
        }
        AbstractBase::createResource(QLatin1String("akonadi_ical_resource"),name,settings);
      } else {
        qDebug()<<" tag unknown :"<<tag;
      }
    }
  } else {
    qDebug()<<" base_uri unknown"<<base_uri;
  }
}
