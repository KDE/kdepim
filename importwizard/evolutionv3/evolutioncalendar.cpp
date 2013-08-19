/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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
#include <QDomDocument>
#include <QDomElement>

EvolutionCalendar::EvolutionCalendar(ImportWizard *parent)
    :AbstractCalendar(parent)
{
}

EvolutionCalendar::~EvolutionCalendar()
{

}

void EvolutionCalendar::loadCalendar(const QString& filename)
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
                    kDebug()<<" attr unknown "<<attr;
                }
            }
        }
    }
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
    //kDebug()<<" info "<<info;
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
                    //TODO: Need to get id for collection to add color.
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
                                } else if( propertyName == QLatin1String("alarm")) {
                                    kDebug()<<" need to implement alarm property";
                                }else {
                                    kDebug()<<" property unknown :"<<propertyName;
                                }
                            }
                        } else {
                            kDebug()<<" tag unknown :"<<propertyTag;
                        }
                    }
                }
                AbstractBase::createResource(QLatin1String("akonadi_ical_resource"),name,settings);
            } else {
                kDebug()<<" tag unknown :"<<tag;
            }
        }
    } else if(base_uri == QLatin1String("webcal://")) {
        kDebug()<<" need to implement webcal protocol";
    } else if(base_uri == QLatin1String("google://")) {
        kDebug()<<" need to implement google protocol";
    } else if(base_uri == QLatin1String("caldav://")) {
        kDebug()<<" need to implement caldav protocol";
    } else {
        kDebug()<<" base_uri unknown"<<base_uri;
    }
}
