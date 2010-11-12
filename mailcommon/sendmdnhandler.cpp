/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#include "sendmdnhandler.h"

#include "mailinterfaces.h"
#include "mailkernel.h"
#include "mailutil.h"
#include "mdnadvicedialog.h"

#include <akonadi/collection.h>
#include <akonadi/item.h>
#include <akonadi/kmime/messageflags.h>
#include <messagecomposer/messagefactory.h>
#include <messagecomposer/messagesender.h>
#include <messagecore/messagehelpers.h>
#include <messageviewer/globalsettings.h>

#include <QtCore/QQueue>
#include <QtCore/QTimer>

using namespace MailCommon;

class SendMdnHandler::Private
{
  public:
    Private( SendMdnHandler *qq, IKernel *kernel )
      : q( qq ), mKernel( kernel )
    {
    }

    void handleMessages();

    SendMdnHandler *q;
    IKernel *mKernel;
    QQueue<Akonadi::Item> mItemQueue;
    QTimer mTimer;
};

void SendMdnHandler::Private::handleMessages()
{
  while ( !mItemQueue.isEmpty() ) {
    Akonadi::Item item = mItemQueue.dequeue();

#if 0
    // should we send an MDN?
    if ( MessageViewer::GlobalSettings::notSendWhenEncrypted() &&
         message()->encryptionState() != KMMsgNotEncrypted &&
         message()->encryptionState() != KMMsgEncryptionStateUnknown )
      return;
#else
    kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif

    const Akonadi::Collection collection = item.parentCollection();
    if ( collection.isValid() &&
         (CommonKernel->folderIsSentMailFolder( collection ) ||
          CommonKernel->folderIsTrash( collection ) ||
          CommonKernel->folderIsDraftOrOutbox( collection ) ||
          CommonKernel->folderIsTemplates( collection )) )
      continue;

    const KMime::Message::Ptr message = MessageCore::Util::message( item );
    if ( !message )
      continue;

    const QPair<bool, KMime::MDN::SendingMode> mdnSend = MDNAdviceHelper::instance()->checkAndSetMDNInfo( item, KMime::MDN::Displayed );
    if ( mdnSend.first ) {
      const KConfigGroup group( mKernel->config(), "MDN" );
      const int quote = group.readEntry<int>( "quote-message", 0 );

      MessageComposer::MessageFactory factory( message, Akonadi::Item().id() );
      factory.setIdentityManager( mKernel->identityManager() );
      factory.setFolderIdentity( MailCommon::Util::folderIdentity( item ) );

      const KMime::Message::Ptr mdn = factory.createMDN( KMime::MDN::ManualAction, KMime::MDN::Displayed, mdnSend.second, quote );
      if ( mdn ) {
        if( !mKernel->msgSender()->send( mdn, MessageSender::SendLater ) ) {
          kDebug() << "Sending failed.";
        }
      }
    }
  }
}


SendMdnHandler::SendMdnHandler( IKernel *kernel, QObject *parent )
  : QObject( parent ), d( new Private( this, kernel ) )
{
  d->mTimer.setSingleShot( true );
  connect( &d->mTimer, SIGNAL( timeout() ), this, SLOT( handleMessages() ) );
}

SendMdnHandler::~SendMdnHandler()
{
  delete d;
}

void SendMdnHandler::setItem( const Akonadi::Item &item )
{
  if ( item.hasFlag( Akonadi::MessageFlags::Seen ) )
    return;

  d->mTimer.stop();

  d->mItemQueue.enqueue( item );

  if ( MessageViewer::GlobalSettings::self()->delayedMarkAsRead() &&
       MessageViewer::GlobalSettings::self()->delayedMarkTime() != 0 ) {
    d->mTimer.start( MessageViewer::GlobalSettings::self()->delayedMarkTime() * 1000 );
    return;
  }

  d->handleMessages();
}

#include "sendmdnhandler.moc"
