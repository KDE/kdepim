/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "../constants.h"
#include "../documentvisitor.h"

#include "category.h"
#include "cloud.h"
#include "constants.h"
#include "document.h"
#include "image.h"
#include "item.h"
#include "textinput.h"

#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>
#include <QList>
#include <QSet>
#include <QString>

#include <krfcdate.h>
#include <kstaticdeleter.h>

namespace LibSyndication {
namespace RSS2 {


Document::Document(const QDomElement& element) : AbstractDocument(), ElementWrapper(element)
{
}

Document Document::fromXML(const QDomDocument& doc)
{
    QDomNode channelNode = doc.namedItem(QString::fromLatin1("rss")).namedItem(QString::fromLatin1("channel"));

    return Document(channelNode.toElement());
}

Document::Document() : AbstractDocument(), ElementWrapper()
{
}

QString Document::title() const
{
    QString t = extractElementText(QString::fromLatin1("title"));
    
    if (!t.isNull())
    {
        return t;
    }
    else
    {
        return extractElementTextNS(LibSyndication::Constants::dublinCoreNamespace(),
                                    QString::fromLatin1("title"));
    }
}

QString Document::link() const
{
    return extractElementText(QString::fromLatin1("link") );
}

QString Document::description() const
{
    QString d = extractElementText(QString::fromLatin1("description"));
    
    if (!d.isNull())
    {
        return d;
    }
    else
    {
        return extractElementTextNS(LibSyndication::Constants::dublinCoreNamespace(),
                                    QString::fromLatin1("description"));
    }

    
}

QString Document::language() const
{
    return extractElementText(QString::fromLatin1("language"));
}

QString Document::copyright() const
{
    QString rights = extractElementText(QString::fromLatin1("copyright"));
    if (!rights.isNull())
    {
        return rights;
    }
    else
    {
        // if <copyright> is not provided, use <dc:rights>
        return extractElementTextNS(
                LibSyndication::Constants::dublinCoreNamespace(), QString::fromLatin1("rights"));
    }
}

QString Document::managingEditor() const
{
    return extractElementText(QString::fromLatin1("managingEditor"));
}

QString Document::webMaster() const
{
    return extractElementText(QString::fromLatin1("webMaster"));
}

QDateTime Document::pubDate() const
{
    QDateTime pubDate;

    QString pubDateStr = extractElementText(QString::fromLatin1("pubDate"));
    
    if (!pubDateStr.isNull())
    {
        time_t time = KRFCDate::parseDate(pubDateStr);
        pubDate.setTime_t(time);
    }
    else
    {   // if there is no pubDate, check for dc:date
        pubDateStr = extractElementTextNS(LibSyndication::Constants::dublinCoreNamespace(), QString::fromLatin1("date"));
        
        if (!pubDateStr.isNull())
        {
            pubDate = QDateTime::fromString(pubDateStr, Qt::ISODate);
        }
    }
    
    return pubDate;
}

QDateTime Document::lastBuildDate() const
{
    QDateTime lastBuildDate;

    QString lastBuildDateStr = extractElementText(QString::fromLatin1("lastBuildDate"));
    if (!lastBuildDateStr.isNull())
    {
        time_t time = KRFCDate::parseDate(lastBuildDateStr);
        lastBuildDate.setTime_t(time);
    }

    return lastBuildDate;
}

QList<Category> Document::categories() const
{
    QList<Category> categories;

    QList<QDomElement> catNodes = elementsByTagName(QString::fromLatin1("category"));
    for (QList<QDomElement>::ConstIterator it = catNodes.begin(); it != catNodes.end(); ++it)
    {
        categories.append(Category(*it));
    }
    
    return categories;
}

QString Document::generator() const
{
    return extractElementText(QString::fromLatin1("generator"));
}

QString Document::docs() const
{
    return extractElementText(QString::fromLatin1("docs"));
}

Cloud Document::cloud() const
{
    return Cloud(element().namedItem(QString::fromLatin1("cloud")).toElement());
}

int Document::ttl() const
{
    bool ok;
    int c;

    QString text = extractElementText(QString::fromLatin1("ttl"));
    c = text.toInt(&ok);
    return ok ? c : 0;
}

Image Document::image() const
{
    QDomNode imageNode = element().namedItem(QString::fromLatin1("image"));
    return Image(imageNode.toElement());
}

TextInput Document::textInput() const
{
    QDomNode textInputNode = element().namedItem(QString::fromLatin1("textInput"));
    return TextInput(textInputNode.toElement());
}

QSet<int> Document::skipHours() const
{
    QSet<int> skipHours;
    QDomNode skipHoursNode = element().namedItem(QString::fromLatin1("skipHours"));
    if (skipHoursNode.isElement())
    {
        ElementWrapper skipHoursWrapper(skipHoursNode.toElement());
        bool ok = false;
        QList<QDomElement> hours = skipHoursWrapper.elementsByTagName(QString::fromLatin1("hour"));
        for (QList<QDomElement>::ConstIterator it = hours.begin(); it != hours.end(); ++it)
        {
            int h = (*it).text().toInt(&ok);
            if (ok)
                skipHours.insert(h);
        }
    }

    return skipHours;
}

QSet<Document::DayOfWeek> Document::skipDays() const
{
    QSet<DayOfWeek> skipDays;
    QDomNode skipDaysNode = element().namedItem(QString::fromLatin1("skipDays"));
    if (skipDaysNode.isElement())
    {
        ElementWrapper skipDaysWrapper(skipDaysNode.toElement());
        QHash<QString, DayOfWeek> weekDays;

        weekDays[QString::fromLatin1("Monday")] = Monday;
        weekDays[QString::fromLatin1("Tuesday")] = Tuesday;
        weekDays[QString::fromLatin1("Wednesday")] = Wednesday;
        weekDays[QString::fromLatin1("Thursday")] = Thursday;
        weekDays[QString::fromLatin1("Friday")] = Friday;
        weekDays[QString::fromLatin1("Saturday")] = Saturday;
        weekDays[QString::fromLatin1("Sunday")] = Sunday;

        QList<QDomElement> days = skipDaysWrapper.elementsByTagName( QString::fromLatin1("day"));
        for (QList<QDomElement>::ConstIterator it = days.begin(); it != days.end(); ++it)
        {
            if (weekDays.contains((*it).text()))
                skipDays.insert(weekDays[(*it).text()]);
        }
    }

    return skipDays;
}

QList<Item> Document::items() const
{
    QList<QDomElement> itemNodes = elementsByTagName(QString::fromLatin1("item"));

    QList<Item> items;

    for (QList<QDomElement>::ConstIterator it = itemNodes.begin(); it != itemNodes.end(); ++it)
    {
        items.append(Item(*it));
    }

    return items;
}

QString Document::debugInfo() const
{
    QString info;
    info += "### Document: ###################\n";
    info += "title: #" + title() + "#\n";
    info += "description: #" + description() + "#\n";
    info += "link: #" + link() + "#\n";
    info += "language: #" + language() + "#\n";
    info += "copyright: #" + copyright() + "#\n";
    info += "managingEditor: #" + managingEditor() + "#\n";
    info += "webMaster: #" + webMaster() + "#\n";
    if (pubDate().isValid())
        info += "pubDate: #" + pubDate().toString() + "#\n";
    info += "lastBuildDate: #" + lastBuildDate().toString() + "#\n";
    if (!textInput().isNull())
        info += textInput().debugInfo();
    if (!cloud().isNull())
        info += cloud().debugInfo();
    if (!image().isNull())
        info += image().debugInfo();
    
    QList<Category> cats = categories();
    
    for (QList<Category>::ConstIterator it = cats.begin(); it != cats.end(); ++it)
        info += (*it).debugInfo();
    QList<Item> litems = items();
    for (QList<Item>::ConstIterator it = litems.begin(); it != litems.end(); ++it)
        info += (*it).debugInfo();
    info += "### Document end ################\n";
    return info;
}

bool Document::accept(DocumentVisitor* visitor)
{
    return visitor->visitRSS2Document(this);
}

} // namespace RSS2
} // namespace LibSyndication
