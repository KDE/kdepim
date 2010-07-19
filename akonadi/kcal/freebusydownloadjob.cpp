/*
    Copyright (C) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "freebusydownloadjob.h"

#include <KDebug>
#include <KIO/Job>
#include <KIO/TransferJob>
#include <KIO/JobUiDelegate>
#include <KJob>

#include <KCal/FreeBusy>

#include "freebusymanager.h"

using namespace Akonadi;
using namespace KCal;

FreeBusyDownloadJob::FreeBusyDownloadJob( const QString &email, const KUrl &url,
                                          FreeBusyManager *manager,
                                          QWidget *parentWidget )
  : QObject( manager ), mManager( manager ), mEmail( email )
{
  KIO::TransferJob *job = KIO::get( url, KIO::NoReload, KIO::HideProgressInfo );

  job->ui()->setWindow( parentWidget );

  connect( job, SIGNAL(result(KJob *)), SLOT(slotResult(KJob *)) );
  connect( job, SIGNAL(data(KIO::Job *,const QByteArray &)),
           SLOT(slotData(KIO::Job *,const QByteArray &)) );
}

FreeBusyDownloadJob::~FreeBusyDownloadJob()
{
}

void FreeBusyDownloadJob::slotData( KIO::Job *, const QByteArray &data )
{
  mFreeBusyData += data;
}

void FreeBusyDownloadJob::slotResult( KJob *job )
{
  kDebug() << mEmail;

  if ( job->error() ) {
    kDebug() << "job error :-(";
  }

  FreeBusy *fb = mManager->iCalToFreeBusy( mFreeBusyData );
  if ( fb ) {
    Person p = fb->organizer();
    p.setEmail( mEmail );
    mManager->saveFreeBusy( fb, p );
  }
  emit freeBusyDownloaded( fb, mEmail );
  deleteLater();
}

#include "freebusydownloadjob.moc"
