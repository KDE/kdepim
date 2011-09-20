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

#ifndef KRSS_FEEDCOLLECTION_H
#define KRSS_FEEDCOLLECTION_H

#include "krss_export.h"

#include <Akonadi/Collection>

namespace KRss {

class KRSS_EXPORT FeedCollection : public Akonadi::Collection
{
public:
    FeedCollection();
    FeedCollection( const Akonadi::Collection &collection );

    static Akonadi::Collection findFolder( const Akonadi::Collection& c );

    bool isFolder() const;

    QString title() const;
    void setTitle( const QString &title );
    QString xmlUrl() const;
    void setXmlUrl( const QString &xmlUrl );
    QString htmlUrl() const;
    void setHtmlUrl( const QString &htmlUrl );
    QString description() const;
    void setDescription( const QString &description );
    QString feedType() const;
    void setFeedType( const QString &feedType );

    int fetchInterval() const;
    void setFetchInterval( int interval );

    bool preferItemLinkForDisplay() const;
    void setPreferItemLinkForDisplay( bool );
};

} // namespace KRss

#endif // KRSS_FEEDCOLLECTION_H
