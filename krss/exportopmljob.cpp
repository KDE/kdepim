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

#include "exportopmljob.h"
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

class ExportOpmlJob::Private {
    ExportOpmlJob* const q;
public:
    explicit Private( const QString& dbusService, ExportOpmlJob* q );

    void doStart();
    void exportFinished( const QVariantMap& );

    KUrl url;
    KTemporaryFile tempFile;
    org::kde::krss* interface;
};

ExportOpmlJob::Private::Private( const QString& dbusService, ExportOpmlJob* qq )
  : q( qq )
  , interface( new org::kde::krss( dbusService,
                                   "/KRss",
                                    QDBusConnection::sessionBus(),
                                    q ) )
{
}

ExportOpmlJob::ExportOpmlJob( const QString& dbusService, QObject* parent ) : KJob( parent ), d( new Private( dbusService, this ) ) {

}

ExportOpmlJob::~ExportOpmlJob() {
    delete d;
}

void ExportOpmlJob::setTargetUrl( const KUrl& url ) {
    d->url = url;
}

void ExportOpmlJob::start() {
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void ExportOpmlJob::Private::doStart() {
    if ( !interface->callWithCallback( "exportOpml", QList<QVariant>() << url.url(), q, SLOT(exportFinished(QVariantMap)) ) ) {
        q->setError( ExportOpmlJob::ExportFailedError );
        q->setErrorText( i18n( "Could not start export." ) );
        q->emitResult();
    }
}

void ExportOpmlJob::Private::exportFinished( const QVariantMap& map ) {
    const int error = map.value( "error" ).toInt();
    if ( error != 0 ) {
        q->setError( ExportOpmlJob::ExportFailedError );
        q->setErrorText( i18n( "Export from RSS resource failed: %1", map.value( "errorString" ).toString() ) );
    }

    q->emitResult();
}

#include "exportopmljob.moc"
