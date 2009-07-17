/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

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

#ifndef KRSS_RESOURCEMANAGER_H
#define KRSS_RESOURCEMANAGER_H

#include "krss_export.h"

#include <QtCore/QStringList>

namespace KRss {

class Resource;
class ResourceManagerPrivate;

class KRSS_EXPORT ResourceManager : public QObject
{
    Q_OBJECT

public:

    static ResourceManager* self();
    ~ResourceManager();

    QStringList identifiers() const;
    QString resourceName( const QString &resourceIdentifier ) const;

    const Resource* resource( const QString &resourceIdentifier );
    static void registerAttributes();

Q_SIGNALS:

    void resourceAdded( const QString &identifier );
    void resourceRemoved( const QString &identifier );

private:

    ResourceManager();
    ResourceManagerPrivate * const d;
};

} //namespace KRss

#endif // KRSS_RESOURCEMANAGER_H
