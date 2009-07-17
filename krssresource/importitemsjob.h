/*
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

#ifndef KRSSRESOURCE_IMPORTITEMSJOB_H
#define KRSSRESOURCE_IMPORTITEMSJOB_H

#include "krssresource_export.h"
#include "itemimportreader.h"

#include <akonadi/item.h>
#include <akonadi/collection.h>

#include <KJob>

#include <QFile>
#include <QString>

namespace KRssResource
{

class KRSSRESOURCE_TESTS_EXPORT ImportItemsJob : public KJob {
    Q_OBJECT
public:

    enum Error {
        CouldNotRetrieveCollection = KJob::UserDefinedError,
        CouldNotOpenFile,
        ItemSyncFailed
    };

    explicit ImportItemsJob( const QString& xmlUrl, QObject *parent = 0 );
    ~ImportItemsJob();

    void setResourceId( const QString& resourceId );
    void setFlagsSynchronizable( bool flagsSynchronizable );
    void setSourceFile( const QString& file );

    /* reimp */ void start();

private:
    void syncItems( const Akonadi::Item::List& items );
    void cleanupAndEmitResult();

private Q_SLOTS:
    void doStart();
    void slotCollectionRetrieved( KJob *job );
    void readBatch();
    void syncDone( KJob* );

private:
    const QString m_xmlUrl;
    QString m_resourceId;
    bool m_flagsSynchronizable;
    Akonadi::Collection m_collection;
    QString m_fileName;
    QFile m_file;
    ItemImportReader* m_reader;
};

} // namespace KRssResource

#endif // KRSSRESOURCE_IMPORTITEMSJOB_H
