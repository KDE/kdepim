/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>
    Copyright (C) 2009    Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KRSS_TAG_P_H
#define KRSS_TAG_P_H

#include "tag.h"
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

namespace KRss {

class TagPrivate : public QSharedData
{
public:
    explicit TagPrivate( const TagId &id )
        : m_id( id )
    {
    }

    TagPrivate( const TagPrivate& other )
        : QSharedData( other ), m_id( other.m_id )
    {
    }

    virtual ~TagPrivate()
    {
    }

    boost::tuple<TagId, QString, QString, QString> asTuple() const
    {
        return boost::make_tuple<TagId, QString, QString, QString>( m_id, label(), description(), icon() );
    }

    virtual QString label() const = 0;
    virtual void setLabel( const QString& label ) = 0;
    virtual QString description() const = 0;
    virtual void setDescription( const QString& description ) = 0;
    virtual QString icon() const = 0;
    virtual void setIcon( const QString& label ) = 0;

public:
    TagId m_id;
};

} // namespace KRss

#endif // KRSS_TAG_P_H
