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

#ifndef KRSSRESOURCE_IMPORTOPMLJOB_H
#define KRSSRESOURCE_IMPORTOPMLJOB_H

#include "opmlparser.h"

#include <krss/feedcollection.h>

#include <KJob>
#include <boost/shared_ptr.hpp>

namespace KRss {
class TagProvider;
}

namespace KRssResource
{
class FeedsImportJob;

class ImportOpmlJob : public KJob
{
    Q_OBJECT
public:
    explicit ImportOpmlJob( const KUrl& path, QObject *parent = 0 );

    void setResourceId( const QString& resourceId );
    void setBackendJob( KRssResource::FeedsImportJob *job );
    void setDefaultTag( const QString& defaultTag );
    QList<Akonadi::Collection> importedFeeds() const;

    void start();
    QString errorString() const;

    enum Error {
        CouldNotReadOpml = KJob::UserDefinedError,
        CouldNotParseOpml,
        CouldNotRetrieveRootCollection,
        CouldNotImportTags,
        CouldNotImportFeeds,
        UserDefinedError
    };

private Q_SLOTS:
    void doStart();
    void slotOpmlRead( KJob *job );
    void slotRootCollectionRetrieved( KJob *job );
    void slotTagProviderRetrieved( KJob *job );
    void slotTagsCreated( KJob *job );
    void slotFeedsImportedBackend( KJob *job );
    void slotFeedImportedAkonadi( KJob *job );

private:
    QString m_resourceId;
    KRssResource::FeedsImportJob *m_backendJob;
    const KUrl m_path;
    QString m_defaultTag;
    KRssResource::OpmlReader m_opmlReader;
    Akonadi::Collection m_rootCollection;
    int m_pendingJobs;
    QList<Akonadi::Collection> m_importedFeeds;
};

class TagsCreateJob : public KJob
{
    Q_OBJECT
public:
    explicit TagsCreateJob( QObject *parent = 0 );

    void setTagProvider( const boost::shared_ptr<const KRss::TagProvider>& tagProvider );
    void setTagLabels( const QList<QString>& tagLabels );

    QList<KRss::Tag> tags() const;

    void start();
    QString errorString() const;

    enum Error {
        CouldNotCreateTags = KJob::UserDefinedError,
        UserDefinedError
    };

private Q_SLOTS:
    void doStart();
    void slotTagCreated( KJob *job );

private:
    boost::shared_ptr<const KRss::TagProvider> m_tagProvider;
    QList<QString> m_tagLabels;
    QVector<KRss::Tag> m_tags;
    int m_pendingJobs;
};

} // namespace KRssResource

#endif // KRSSRESOURCE_IMPORTOPMLJOB_H
