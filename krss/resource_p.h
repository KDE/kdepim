/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

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

#ifndef KRSS_RESOURCE_P_H
#define KRSS_RESOURCE_P_H

#include "feed.h"

#include <QtCore/QHash>
#include <QtCore/QPointer>

namespace KRss {

class ResourcePrivate
{
public:
    explicit ResourcePrivate( const QString& resourceId, const QString& name )
        : m_id( resourceId ), m_name( name ) {};
    virtual ~ResourcePrivate();

    const QString m_id;
    const QString m_name;
    QHash<Feed::Id, QPointer<Feed> > m_feeds;
};

} // namespace KRss

#endif // KRSS_RESOURCE_P_H
