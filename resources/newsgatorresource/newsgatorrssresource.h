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

#ifndef KRSS_NEWSGATORRESOURCE_NEWSGATORRSSRESOURCE_H
#define KRSS_NEWSGATORRESOURCE_NEWSGATORRSSRESOURCE_H

#include <krssresource/rssresourcebase.h>

#include "location.h"

class NewsgatorRssResource : public KRssResource::RssResourceBase
{
    Q_OBJECT
public:
    explicit NewsgatorRssResource( const QString &id );

public Q_SLOTS:
    void configure( WId windowId );

private Q_SLOTS:
    void slotReloadConfiguration();

private:
    void loadConfiguration();

private:    // impl
    bool flagsSynchronizable() const;
    KRssResource::FeedsRetrieveJob* feedsRetrieveJob() const;
    KRssResource::FeedsImportJob* feedsImportJob() const;
    KRssResource::FeedCreateJob* feedCreateJob() const;
    KRssResource::FeedModifyJob* feedModifyJob() const;
    KRssResource::FeedDeleteJob* feedDeleteJob() const;
    KRssResource::FeedFetchJob* feedFetchJob() const;
    KRssResource::ItemModifyJob* itemModifyJob() const;

private:
    QString m_userName;
    QString m_password;
    Location m_location;
};

#endif // KRSS_NEWSGATORRESOURCE_NEWSGATORRSSRESOURCE_H
