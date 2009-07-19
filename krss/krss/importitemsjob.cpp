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

#include "importitemsjob.h"
#include "krssinterface.h"
#include "resource.h"
#include "dbushelper_p.h"

#include <KLocalizedString>

#include <cassert>

using namespace KRss;

class ImportItemsJob::Private {
    ImportItemsJob* const q;
public:
    explicit Private( const QString& dbusService, ImportItemsJob* q );

    void doStart();
    void importFinished( const QVariantMap& );
    void importError( const QDBusError& err );

    QString url;
    QString sourceFile;
    org::kde::krss* interface;
};

ImportItemsJob::Private::Private( const QString& dbusService, ImportItemsJob* qq )
  : q( qq )
  , interface( new org::kde::krss( dbusService,
                                   "/KRss",
                                    QDBusConnection::sessionBus(),
                                    q ) )
{
}


ImportItemsJob::ImportItemsJob( const QString& dbusService, QObject* parent ) : KJob( parent ), d( new Private( dbusService, this ) ) {

}

ImportItemsJob::~ImportItemsJob() {
    delete d;
}

void ImportItemsJob::setFeedXmlUrl( const QString& url ) {
    d->url = url;
}

void ImportItemsJob::setSourceFile( const QString& path ) {
    d->sourceFile = path;
}

void ImportItemsJob::start() {
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void ImportItemsJob::Private::doStart() {
    if ( !DBusHelper::callWithCallback( interface, "importItems", QList<QVariant>() << url << sourceFile, q, SLOT(importFinished(QVariantMap)), SLOT(importError(QDBusError)), DBusHelper::NoTimeout ) ) {
        q->setError( ImportItemsJob::ImportFailedError );
        q->setErrorText( i18n( "Could not start import." ) );
        q->emitResult();
    }
}

void ImportItemsJob::Private::importFinished( const QVariantMap& map ) {
    if ( map.value( "error" ).toInt() != 0 ) {
        q->setError( ImportItemsJob::ImportFailedError );
        q->setErrorText( map.value( "errorString" ).toString() );
    }
    q->emitResult();
}

void ImportItemsJob::Private::importError( const QDBusError& err ) {
    q->setError( ImportItemsJob::ImportFailedError );
    q->setErrorText( i18n( "Item import failed: %1, %2" ).arg( err.name(), err.message() ) );
    q->emitResult();
}

#include "importitemsjob.moc"

