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

#include "importfeedlistcommand.h"
#include "command_p.h"

#include <Akonadi/Collection>
#include <Akonadi/Session>

#include <krss/importopmljob.h>

#include <KFileDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrl>

#include <QTimer>

#include <boost/shared_ptr.hpp>

#include <cassert>

using namespace Akonadi;
using namespace Akregator;
using namespace KRss;

class ImportFeedListCommand::Private
{
    ImportFeedListCommand* const q;
public:
    explicit Private( ImportFeedListCommand* qq );

    void doImport();
    void importFinished( KJob* );

    KUrl url;
    Collection targetCollection;
    Session* session;
};

ImportFeedListCommand::Private::Private( ImportFeedListCommand* qq )
    : q( qq )
    , session( 0 )
{

}

void ImportFeedListCommand::Private::doImport()
{
    Q_ASSERT( session );
    if ( !targetCollection.isValid() || !targetCollection.contentMimeTypes().contains( Collection::mimeType() ) ) {
        //PENDING(frank) offer chooser dialog instead
        q->setErrorAndEmitResult( i18n("Please select a folder to add the feeds to.") );
        return;
    }

    EmitResultGuard guard( q );
    if ( !url.isValid() ) {
        url = KFileDialog::getOpenUrl( KUrl(),
                                       QLatin1String("*.opml *.xml|")
                                       + i18n("OPML Outlines (*.opml, *.xml)")
                                       + QLatin1String("\n*|") + i18n("All Files"),
                                       q->parentWidget(), i18n("Feed List Import") );
        if ( !guard.exists() )
            return;
    }

    if ( !url.isValid() ) {
        guard.emitCanceled();
        return;
    }
#ifdef KRSS_PORT_DISABLED
    KRss::ImportOpmlJob* job = resource->createImportOpmlJob( url );
    connect( job, SIGNAL(finished(KJob*)), q, SLOT(importFinished(KJob*)) );
    job->start();
#endif
}

void ImportFeedListCommand::Private::importFinished( KJob* job ) {
    EmitResultGuard guard( q );
    if ( job->error() ) {
        q->setError( Command::SomeError );
        q->setErrorText( i18n("Could not import feed list: %1", job->errorString() ) );
    }
    else
        KMessageBox::information( q->parentWidget(), i18n("The feed list was successfully imported." ), i18n("Import Finished") );
    guard.emitResult();
}

ImportFeedListCommand::ImportFeedListCommand( QObject* parent ) : Command( parent ), d( new Private( this ) )
{
}

ImportFeedListCommand::~ImportFeedListCommand()
{
    delete d;
}

void ImportFeedListCommand::setSourceUrl( const KUrl& url )
{
    d->url = url;
}

void ImportFeedListCommand::setSession( Session* s )
{
    d->session = s;
}


void ImportFeedListCommand::setTargetCollection( const Collection& c )
{
    d->targetCollection = c;
}

void ImportFeedListCommand::doStart()
{
    d->doImport();
}

#include "importfeedlistcommand.moc"
