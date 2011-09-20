/*
    This file is part of Akregator.

    Copyright (C) 2009 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "exportfeedlistcommand.h"
#include "command_p.h"

#include <krss/exportopmljob.h>
#include <krss/feedcollection.h>

#include <KFileDialog>
#include <KInputDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrl>

#include <QTimer>

using namespace Akonadi;
using namespace Akregator;
using namespace KRss;

class ExportFeedListCommand::Private
{
    ExportFeedListCommand* const q;
public:
    explicit Private( ExportFeedListCommand* qq );

    void doExport();
    void exportFinished( KJob* );

    KUrl url;
    Akonadi::Session* session;
    Collection::List rootCollections;
    Collection preset;
};

ExportFeedListCommand::Private::Private( ExportFeedListCommand* qq )
    : q( qq )
    , session( 0 )
{
    q->setUserVisible( true );
    q->setShowErrorDialog( true );
}

void ExportFeedListCommand::Private::doExport()
{
    EmitResultGuard guard( q );

    Collection selected = preset;
    QHash<QString,int> occs;
    QStringList titles;
    titles.reserve( rootCollections.size() );
    Q_FOREACH( const Collection& i, rootCollections ) {
        const QString t = FeedCollection( i ).title();
        //not efficient, but who has trillions of resources anyway? Right? Right.
        if ( titles.contains( t ) ) {
            occs[t] += 1;
            titles.append( i18nc( "folder title (occurrence number, for duplicates)", "%1 (%2)", t, QString::number( occs[t] + 1 ) ) );
        }
        else
            titles.append( t );
    }

    if ( rootCollections.size() > 1 ) {
        bool ok;
        const QString sel =
                KInputDialog::getItem( i18n("Feed List Export"),
                                       i18n("Please select the list to export"),
                                       titles,
                                       rootCollections.indexOf( selected ),
                                       /*editable=*/false,
                                       &ok,
                                       q->parentWidget() );
        if ( !ok ) {
            guard.emitCanceled();
            return;
        }

        if ( !guard.exists() )
            return;

        selected = rootCollections.at( titles.indexOf( sel ) );
    }

    Q_ASSERT( selected.isValid() );

    if ( !url.isValid() ) {
        url = KFileDialog::getSaveUrl( KUrl(),
                            QLatin1String("*.opml *.xml|") + i18n("OPML Outlines (*.opml, *.xml)")
                            + QLatin1String("\n*|") + i18n("All Files") );
        if ( !guard.exists() )
           return;
    }

    if ( !url.isValid() ) {
        guard.emitCanceled();
        return;
    }
#ifdef KRSS_PORT_DISABLED
    KRss::ExportOpmlJob* job = resource->createExportOpmlJob( url );
    connect( job, SIGNAL(finished(KJob*)), q, SLOT(exportFinished(KJob*)) );
    job->start();
#endif
}

void ExportFeedListCommand::Private::exportFinished( KJob* job ) {
    if ( job->error() )
        q->setErrorAndEmitResult( i18n("Could not export feed list: %1", job->errorString() ) );
    else
        q->emitResult();
}

ExportFeedListCommand::ExportFeedListCommand( QObject* parent ) : Command( parent ), d( new Private( this ) )
{
}

ExportFeedListCommand::~ExportFeedListCommand()
{
    delete d;
}

void ExportFeedListCommand::setTargetUrl( const KUrl& url )
{
    d->url = url;
}

void ExportFeedListCommand::setSession( Akonadi::Session* s )
{
    d->session = s;
}

void ExportFeedListCommand::setRootCollections(const Akonadi::Collection::List& list, const Akonadi::Collection& preset ) {
    Q_ASSERT( !preset.isValid() || list.contains( preset ) );
    d->rootCollections = list;
    if( !list.isEmpty() )
        d->preset = preset.isValid() ? preset : d->rootCollections.first();
    else
        d->preset = Collection();
}

void ExportFeedListCommand::doStart()
{
    Q_ASSERT( d->session );
    d->doExport();
}

#include "exportfeedlistcommand.moc"
