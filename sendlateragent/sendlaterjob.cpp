/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "sendlaterjob.h"
#include "sendlaterinfo.h"
#include "sendlatermanager.h"

#include <mailtransport/transportattribute.h>
#include <mailtransport/sentbehaviourattribute.h>

#include <Akonadi/ItemFetchJob>

#include <KNotification>
#include <KLocale>
#include <KGlobal>
#include <KIcon>
#include <KIconLoader>

SendLaterJob::SendLaterJob(SendLaterManager *manager, SendLaterInfo *info, QObject *parent)
    : QObject(parent),
      mManager(manager),
      mInfo(info)
{
}

SendLaterJob::~SendLaterJob()
{

}

void SendLaterJob::start()
{
    if (mInfo) {
        if (mInfo->itemId() > -1) {
            const Akonadi::Item item = Akonadi::Item(mInfo->itemId());
            Akonadi::ItemFetchJob *fetch = new Akonadi::ItemFetchJob( item, this );
            mFetchScope.fetchAttribute<MailTransport::TransportAttribute>();
            mFetchScope.fetchAttribute<MailTransport::SentBehaviourAttribute>();
            mFetchScope.setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
            fetch->setFetchScope( mFetchScope );
            connect( fetch, SIGNAL(itemsReceived(Akonadi::Item::List)), SLOT(slotMessageTransfered(Akonadi::Item::List)) );
            connect( fetch, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)) );
            fetch->start();
        }
    }
}

void SendLaterJob::slotMessageTransfered(const Akonadi::Item::List& items)
{
    if (items.count() == 1) {
        //Success
        mItem = items.first();
        return;
    }
    //TODO error
}

void SendLaterJob::slotJobFinished(KJob* job)
{
    if ( job->error() ) {
        sendError(i18n("Can not fetch message. %1", job->errorString() ));
        kDebug()<<"Can not fetch message: "<<job->errorString();
        return;
    }
    if (mItem.isValid()) {
        //Send it :)
    }
    //TODO
}

void SendLaterJob::sendDone()
{
    const QPixmap pixmap = KIcon( QLatin1String("kmail") ).pixmap( KIconLoader::SizeSmall, KIconLoader::SizeSmall );

    KNotification::event( QLatin1String("mailsend"),
                          QString(), /*TODO*/
                          pixmap,
                          0,
                          KNotification::CloseOnTimeout,
                          KGlobal::mainComponent());
    mManager->sendDone(mInfo);
}

void SendLaterJob::sendError(const QString &error)
{
    const QPixmap pixmap = KIcon( QLatin1String("kmail") ).pixmap( KIconLoader::SizeSmall, KIconLoader::SizeSmall );
    KNotification::event( QLatin1String("mailsendfailed"),
                          error,
                          pixmap,
                          0,
                          KNotification::CloseOnTimeout,
                          KGlobal::mainComponent());
    mManager->sendError(mInfo);
}

#include "sendlaterjob.moc"
