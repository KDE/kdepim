/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#include "dublincore.h"
#include "dublincorevocab.h"
#include "property.h"
#include "statement.h"

#include <tools.h>

#include <QList>
#include <QString>
#include <QStringList>

namespace Syndication {
namespace RDF {
    


DublinCore::DublinCore(ResourcePtr resource) : ResourceWrapper(resource)
{
}

DublinCore::~DublinCore()
{
}

QString DublinCore::contributor() const
{
    return resource()->property(DublinCoreVocab::self()->contributor())->asString();
}

QStringList DublinCore::contributors() const
{
    QStringList res;
    QList<StatementPtr> list = resource()->properties(DublinCoreVocab::self()->contributor());
    QList<StatementPtr>::ConstIterator it = list.begin();
    QList<StatementPtr>::ConstIterator end = list.end();
    for ( ; it != end; ++it)
    {
        QString str = (*it)->asString();
        if (!str.isNull())
            res.append(str);
    }
    return res;
}

QString DublinCore::coverage() const
{
    return resource()->property(DublinCoreVocab::self()->coverage())->asString();
}

QString DublinCore::creator() const
{
    return resource()->property(DublinCoreVocab::self()->creator())->asString();
}

QStringList DublinCore::creators() const
{
    QStringList res;
    QList<StatementPtr> list = resource()->properties(DublinCoreVocab::self()->creator());
    QList<StatementPtr>::ConstIterator it = list.begin();
    QList<StatementPtr>::ConstIterator end = list.end();
    for ( ; it != end; ++it)
    {
        QString str = (*it)->asString();
        if (!str.isNull())
            res.append(str);
    }
    return res;
}

time_t DublinCore::date() const
{
    QString str =  resource()->property(DublinCoreVocab::self()->date())->asString();
    return parseDate(str, ISODate);
    
}

QString DublinCore::description() const
{
    return resource()->property(DublinCoreVocab::self()->description())->asString();
}

QString DublinCore::format() const
{
    return resource()->property(DublinCoreVocab::self()->format())->asString();
}

QString DublinCore::identifier() const
{
    return resource()->property(DublinCoreVocab::self()->identifier())->asString();
}

QString DublinCore::language() const
{
    return resource()->property(DublinCoreVocab::self()->language())->asString();
}

QString DublinCore::publisher() const
{
    return resource()->property(DublinCoreVocab::self()->publisher())->asString();
}

QString DublinCore::relation() const
{
    return resource()->property(DublinCoreVocab::self()->relation())->asString();
}

QString DublinCore::rights() const
{
    return resource()->property(DublinCoreVocab::self()->rights())->asString();
}

QString DublinCore::source() const
{
    return resource()->property(DublinCoreVocab::self()->source())->asString();
}

QString DublinCore::subject() const
{
    return resource()->property(DublinCoreVocab::self()->subject())->asString();
}

QStringList DublinCore::subjects() const
{
    QStringList res;
    QList<StatementPtr> list = resource()->properties(DublinCoreVocab::self()->subject());
    QList<StatementPtr>::ConstIterator it = list.begin();
    QList<StatementPtr>::ConstIterator end = list.end();
    for ( ; it != end; ++it)
    {
        QString str = (*it)->asString();
        if (!str.isNull())
            res.append(str);
    }
    return res;
}

QString DublinCore::title() const
{
    return resource()->property(DublinCoreVocab::self()->title())->asString();
}

QString DublinCore::type() const
{
    return resource()->property(DublinCoreVocab::self()->type())->asString();
}

QString DublinCore::debugInfo() const
{
    QString info;
    if (!contributor().isNull())
        info += QString("dc:contributor: #%1#\n").arg(contributor());
    if (!coverage().isNull())
        info += QString("dc:coverage: #%1#\n").arg(coverage());
    if (!creator().isNull())
        info += QString("dc:creator: #%1#\n").arg(creator());
    
    
    QString ddate = dateTimeToString(date());
    if (!ddate.isNull())
        info += QString("dc:date: #%1#\n").arg(ddate);
    
    if (!description().isNull())
        info += QString("dc:description: #%1#\n").arg(description());
    if (!format().isNull())
        info += QString("dc:format: #%1#\n").arg(format());
    if (!identifier().isNull())
        info += QString("dc:identifier: #%1#\n").arg(identifier());
    if (!language().isNull())
        info += QString("dc:language: #%1#\n").arg(language());
    if (!publisher().isNull())
        info += QString("dc:publisher: #%1#\n").arg(publisher());
    if (!relation().isNull())
        info += QString("dc:relation: #%1#\n").arg(relation());
    if (!rights().isNull())
        info += QString("dc:rights: #%1#\n").arg(rights());
    if (!source().isNull())
        info += QString("dc:source: #%1#\n").arg(source());
    if (!subject().isNull())
        info += QString("dc:subject: #%1#\n").arg(subject());
    if (!title().isNull())
        info += QString("dc:title: #%1#\n").arg(title());
    if (!type().isNull())
        info += QString("dc:type: #%1#\n").arg(type());
    return info;
}

} // namespace RDF
} // namespace Syndication

