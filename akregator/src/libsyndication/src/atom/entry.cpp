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

#include "category.h"
#include "constants.h"
#include "content.h"
#include "entry.h"
#include "link.h"
#include "person.h"
#include "source.h"
#include "tools.h"

#include <QDateTime>
#include <QDomElement>
#include <QList>
#include <QString>

namespace LibSyndication {
namespace Atom {

Entry::Entry() : ElementWrapper()
{
}

Entry::Entry(const QDomElement& element) : ElementWrapper(element)
{
}

QList<Person> Entry::authors() const
{
    QList<QDomElement> a = 
            Tools::elementsByTagNameNS(element(), Constants::atom1NameSpace(), 
                                       QString::fromLatin1("author"));
    QList<Person> list;
                                       
    QList<QDomElement>::ConstIterator it = a.begin();
    QList<QDomElement>::ConstIterator end = a.end();
    
    
    for ( ; it != end; ++it)
    {
        list.append(Person(*it));
    }
        
    return list;
}

QList<Person> Entry::contributors() const
{
    QList<QDomElement> a = 
            Tools::elementsByTagNameNS(element(), Constants::atom1NameSpace(), 
                                       QString::fromLatin1("contributor"));
    QList<Person> list;
                                       
    QList<QDomElement>::ConstIterator it = a.begin();
    QList<QDomElement>::ConstIterator end = a.end();
    
    
    for ( ; it != end; ++it)
    {
        list.append(Person(*it));
    }
        
    return list;
}

QList<Category> Entry::categories() const
{
    QList<QDomElement> a = 
            Tools::elementsByTagNameNS(element(), Constants::atom1NameSpace(), 
                                       QString::fromLatin1("category"));
    QList<Category> list;
    
    QList<QDomElement>::ConstIterator it = a.begin();
    QList<QDomElement>::ConstIterator end = a.end();


    for ( ; it != end; ++it)
    {
        list.append(Category(*it));
    }

    return list;
}

QString Entry::id() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("id"));

}

QList<Link> Entry::links() const
{
    QList<QDomElement> a = 
            Tools::elementsByTagNameNS(element(), Constants::atom1NameSpace(), 
                                       QString::fromLatin1("link"));
    QList<Link> list;
    
    QList<QDomElement>::ConstIterator it = a.begin();
    QList<QDomElement>::ConstIterator end = a.end();


    for ( ; it != end; ++it)
    {
        list.append(Link(*it));
    }

    return list;
}

QString Entry::rights() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("rights"));
}

Source Entry::source() const
{
    return Source(Tools::firstElementByTagNameNS(element(),
                   Constants::atom1NameSpace(),
                   QString::fromLatin1("source")));
}

time_t Entry::published() const
{
    QString pub = Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("published"));
    QDateTime dt = QDateTime::fromString(pub, Qt::ISODate);
    return dt.toTime_t();
}

time_t Entry::updated() const
{
    QString upd = Tools::extractElementTextNS(element(), 
                                              Constants::atom1NameSpace(),
                                              QString::fromLatin1("published"));
    
    QDateTime dt = QDateTime::fromString(upd, Qt::ISODate);
    return dt.toTime_t();
}

QString Entry::summary() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("summary"));
}

QString Entry::title() const
{
    return Tools::extractElementTextNS(element(), 
                                       Constants::atom1NameSpace(),
                                       QString::fromLatin1("title"));
}

Content Entry::content() const
{
    return Content(Tools::firstElementByTagNameNS(element(),
                   Constants::atom1NameSpace(),
                   QString::fromLatin1("content")));
}

QString Entry::debugInfo() const
{
    QString info;
    info += "### Entry: ###################\n";
    if (!title().isEmpty())
        info += "title: #" + title() + "#\n";
    if (!summary().isEmpty())
        info += "summary: #" + summary() + "#\n";
    if (!id().isEmpty())
        info += "id: #" + id() + "#\n";
    if (!content().isNull())
        info += content().debugInfo();
    
    if (!rights().isEmpty())
        info += "rights: #" + rights() + "#\n";
    
    QDateTime dupdated;
    dupdated.setTime_t(updated()); 
    info += "updated: #" + dupdated.toString(Qt::ISODate) + "#\n";
    
    QDateTime dpublished;
    dpublished.setTime_t(published());
    info += "published: #" + dpublished.toString(Qt::ISODate) + "#\n";
    
    QList<Link> dlinks = links();
    QList<Link>::ConstIterator endlinks = dlinks.end();
    for (QList<Link>::ConstIterator it = dlinks.begin(); it != endlinks; ++it)
        info += (*it).debugInfo();
    
    QList<Category> dcats = categories();
    QList<Category>::ConstIterator endcats = dcats.end();
    for (QList<Category>::ConstIterator it = dcats.begin(); it != endcats; ++it)
        info += (*it).debugInfo();

    info += "### Authors: ###################\n";
    
    QList<Person> dauthors = authors();
    QList<Person>::ConstIterator endauthors = dauthors.end();
    for (QList<Person>::ConstIterator it = dauthors.begin(); it != endauthors; ++it)
        info += (*it).debugInfo();

    info += "### Contributors: ###################\n";
    
    QList<Person> dcontri = contributors();
    QList<Person>::ConstIterator endcontri = dcontri.end();
    for (QList<Person>::ConstIterator it = dcontri.begin(); it != endcontri; ++it)
        info += (*it).debugInfo();
    
    if (!source().isNull())
        info += source().debugInfo();
    
    info += "### Entry end ################\n";

    return info;
}

} // namespace Atom
} //namespace LibSyndication

