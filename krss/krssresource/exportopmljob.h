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

#ifndef KRSSRESOURCE_EXPORTOPMLJOB_H
#define KRSSRESOURCE_EXPORTOPMLJOB_H

#include <KJob>
#include <KUrl>

namespace KRssResource
{

class ExportOpmlJob : public KJob
{
    Q_OBJECT
public:
    explicit ExportOpmlJob( const KUrl& path, QObject *parent = 0 );

    void setResourceId( const QString& resourceId );
    void start();
    QString errorString() const;

    enum Error {
        CouldNotRetrieveCollections = KJob::UserDefinedError,
        CouldNotRetrieveTagProvider,
        CouldNotExportFeeds,
        UserDefinedError
    };

private Q_SLOTS:
    void doStart();
    void slotCollectionsRetrieved( KJob *job );
    void slotTagProviderRetrieved( KJob *job );
    void slotPutFinished( KJob *job );

private:
    QString m_resourceId;
    const KUrl m_path;
};

} // namespace KRssResource

#endif // KRSSRESOURCE_EXPORTOPMLJOB_H
