/*
 * This file is part of the krss library
 *
 * Copyright (C) 2009 Frank Osterfeld <osterfeld@kde.org>
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

#include "importopmljob.h"
#include "krssinterface.h"
#include "resource.h"

#include <kio/job.h>
#include <kio/jobclasses.h>

#include <KLocalizedString>
#include <KTemporaryFile>
#include <KUrl>

#include <cerrno>
#include <cassert>

using namespace KIO;
using namespace KRss;

class ImportOpmlJob::Private {
    ImportOpmlJob* const q;
public:
    explicit Private( const QString& dbusService, ImportOpmlJob* q );

    void doStart();
    void importFinished( const QVariantMap& );

    KUrl url;
    KTemporaryFile tempFile;
    org::kde::krss* interface;
    QList<ImportOpmlJob::FeedInfo> feedInfos;
};

ImportOpmlJob::Private::Private( const QString& dbusService, ImportOpmlJob* qq )
  : q( qq )
  , interface( new org::kde::krss( dbusService,
                                   "/KRss",
                                    QDBusConnection::sessionBus(),
                                    q ) )
{
}

ImportOpmlJob::ImportOpmlJob( const QString& dbusService, QObject* parent ) : KJob( parent ), d( new Private( dbusService, this ) ) {

}

ImportOpmlJob::~ImportOpmlJob() {
    delete d;
}

void ImportOpmlJob::setSourceUrl( const KUrl& url ) {
    d->url = url;
}

QList<ImportOpmlJob::FeedInfo> ImportOpmlJob::importedFeeds() const {
    return d->feedInfos;
}

void ImportOpmlJob::start() {
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void ImportOpmlJob::Private::doStart() {
    if ( !interface->callWithCallback( "importOpml", QList<QVariant>() << url.url() << QString(), q, SLOT(importFinished(QVariantMap)) ) ) {
        q->setError( ImportOpmlJob::ImportFailedError );
        q->setErrorText( i18n( "Could not start import." ) );
        q->emitResult();
    }
}

void ImportOpmlJob::Private::importFinished( const QVariantMap& map ) {
    const int error = map.value( "error" ).toInt();
    if ( error != 0 ) {
        q->setError( ImportOpmlJob::ImportFailedError );
        q->setErrorText( i18n( "Import into RSS resource failed: %1", map.value( "errorString" ).toString() ) );
    }
    const QStringList feedUrls = map.value( "feedUrls" ).toStringList();
    const QStringList feedTitles = map.value( "feedTitles" ).toStringList();

    assert( feedUrls.size() == feedTitles.size() );
    for ( int i = 0; i < feedUrls.size(); ++i ) {
        ImportOpmlJob::FeedInfo info;
        info.xmlUrl = feedUrls[i];
        info.title = feedTitles[i];
        feedInfos.append( info );
    }

    q->emitResult();
}

#include "importopmljob.moc"
