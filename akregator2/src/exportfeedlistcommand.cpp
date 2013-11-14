/*
    This file is part of Akregator2.

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

#include <Akonadi/AgentManager>
#include <KRss/ExportToOpmlJob>
#include <KFileDialog>
#include <KInputDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrl>

#include <QTimer>


using namespace Akonadi;
using namespace Akregator2;
using namespace KRss;

class ExportFeedListCommand::Private
{
    ExportFeedListCommand* const q;
public:
    explicit Private( ExportFeedListCommand* qq );

    void doExport();
    void exportFinished( KJob* );

    QString outputFile;
    Akonadi::Session* session;
    QString resourceIdentifier;
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

    if ( resourceIdentifier.isEmpty() ) {
        //TODO
    }


    if ( outputFile.isEmpty() ) {
        outputFile = KFileDialog::getSaveFileName( KUrl(),
                            QLatin1String("*.opml *.xml|") + i18n("OPML Outlines (*.opml, *.xml)")
                            + QLatin1String("\n*|") + i18n("All Files"), q->parentWidget() );
        if ( !guard.exists() )
           return;
    }

    if ( outputFile.isEmpty() ) {
        guard.emitCanceled();
        return;
    }

    ExportToOpmlJob* job = new ExportToOpmlJob( q );
    job->setResource( resourceIdentifier );
    job->setOutputFile( outputFile );
    job->setIncludeCustomProperties( false );
    connect( job, SIGNAL(finished(KJob*)), q, SLOT(exportFinished(KJob*)) );
    job->start();
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

void ExportFeedListCommand::setOutputFile( const QString& outputFile )
{
    d->outputFile = outputFile;
}

void ExportFeedListCommand::setSession( Akonadi::Session* s )
{
    d->session = s;
}

void ExportFeedListCommand::setResource( const QString& identifier )
{
    d->resourceIdentifier = identifier;
}

void ExportFeedListCommand::doStart()
{
    Q_ASSERT( d->session );
    d->doExport();
}

