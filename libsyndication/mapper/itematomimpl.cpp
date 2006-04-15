/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "categoryatomimpl.h"
#include "enclosureatomimpl.h"
#include "itematomimpl.h"

#include <atom/category.h>
#include <atom/content.h>
#include <atom/link.h>
#include <atom/person.h>
#include <category.h>
#include <constants.h>
#include <enclosure.h>
#include <personimpl.h>
#include <tools.h>

#include <QList>
#include <QString>


using Syndication::Atom::Content;
using Syndication::Atom::Link;
using Syndication::Atom::Person;

namespace Syndication {

ItemAtomImpl::ItemAtomImpl(const Syndication::Atom::Entry& entry) : m_entry(entry)
{
}

QString ItemAtomImpl::title() const
{
    return m_entry.title();
}

QString ItemAtomImpl::link() const
{
    QList<Syndication::Atom::Link> links = m_entry.links();
    QList<Syndication::Atom::Link>::ConstIterator it = links.begin();
    QList<Syndication::Atom::Link>::ConstIterator end = links.end();

    // return first link where rel="alternate"
    for ( ; it != end; ++it)
    {
        if ((*it).rel() == QString::fromUtf8("alternate"))
        {
            return (*it).href();
        }
    }
    
    return QString::null;
}

QString ItemAtomImpl::description() const
{
    return m_entry.summary();
}

QString ItemAtomImpl::content() const
{
    Content content = m_entry.content();
    if (content.isNull())
        return QString::null;
    
    return content.asString();
}

QList<PersonPtr> ItemAtomImpl::authors() const
{
    QList<Syndication::Atom::Person> atomps = m_entry.authors();
    QList<Syndication::Atom::Person>::ConstIterator it = atomps.begin();
    QList<Syndication::Atom::Person>::ConstIterator end = atomps.end();
    
    QList<PersonPtr> list;
    
    for ( ; it != end; ++it)
    {
        PersonImplPtr ptr(new PersonImpl((*it).name(), (*it).uri(), (*it).email()));
        list.append(ptr);
    }
    
    atomps = m_entry.contributors();
    
    it = atomps.begin();
    end = atomps.end();
    
    for ( ; it != end; ++it)
    {
        PersonImplPtr ptr(new PersonImpl((*it).name(), (*it).uri(), (*it).email()));
        list.append(ptr);
    }
    
    return list;
}

time_t ItemAtomImpl::datePublished() const 
{
    return m_entry.published();
}

time_t ItemAtomImpl::dateUpdated() const 
{
    time_t upd = m_entry.updated();
    if (upd == 0)
        return m_entry.published();
    else
        return upd;
}
   
QString ItemAtomImpl::language() const
{
    return m_entry.xmlLang();
}

QString ItemAtomImpl::id() const
{
    QString id = m_entry.id();
    if (!id.isEmpty())
        return id;
    
    return QString("hash:%1").arg(Syndication::calcMD5Sum(title() + description() + link() + content()));
}


QList<Syndication::EnclosurePtr> ItemAtomImpl::enclosures() const
{
    QList<Syndication::EnclosurePtr> list;

    QList<Syndication::Atom::Link> links = m_entry.links();
    QList<Syndication::Atom::Link>::ConstIterator it = links.begin();
    QList<Syndication::Atom::Link>::ConstIterator end = links.end();

    for ( ; it != end; ++it)
    {
        if ((*it).rel() == QString::fromUtf8("enclosure"))
        {
            list.append(EnclosureAtomImplPtr(new EnclosureAtomImpl(*it)));

        }
    }

    return list;
}

QList<Syndication::CategoryPtr> ItemAtomImpl::categories() const
{
    QList<Syndication::CategoryPtr> list;
    
    QList<Syndication::Atom::Category> cats = m_entry.categories();
    QList<Syndication::Atom::Category>::ConstIterator it = cats.begin();
    QList<Syndication::Atom::Category>::ConstIterator end = cats.end();
    
    for ( ; it != end; ++it)
    {
        CategoryAtomImplPtr impl(new CategoryAtomImpl(*it));
        list.append(impl);
    }
    
    return list;
}

int ItemAtomImpl::commentsCount() const
{
    QString cstr = m_entry.extractElementTextNS(slashNamespace(), QString::fromUtf8("comments"));
    bool ok = false;
    int comments = cstr.toInt(&ok);
    return ok ? comments : -1;
}

QString ItemAtomImpl::commentsLink() const
{
    return QString();
}

QString ItemAtomImpl::commentsFeed() const
{
    return m_entry.extractElementTextNS(commentApiNamespace(), QString::fromUtf8("commentRss"));
}

QString ItemAtomImpl::commentPostUri() const
{
    return m_entry.extractElementTextNS(commentApiNamespace(), QString::fromUtf8("comment"));
}

Syndication::SpecificItemPtr ItemAtomImpl::specificItem() const
{
    return Syndication::SpecificItemPtr(new Syndication::Atom::Entry(m_entry));
}

} // namespace Syndication
