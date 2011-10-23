/*
 * This file is part of KMail.
 * Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "akonadisender.h"

#include <KLocale>
#include <KDebug>

#include "messagehelper.h"
#include "messagecomposersettings.h"

#include <kmime/kmime_message.h>
#include <boost/shared_ptr.hpp>

#include <progressmanager.h>

#include <mailtransport/dispatcherinterface.h>
#include <mailtransport/messagequeuejob.h>
#include <mailtransport/transport.h>
#include <mailtransport/transportmanager.h>
#include <messagecore/stringutil.h>
#include <messagecore/messagehelpers.h>

using namespace KMime;
using namespace KMime::Types;
using namespace KPIM;
using namespace MailTransport;


static QStringList addrSpecListToStringList( const AddrSpecList &l, bool allowEmpty = false )
{
  QStringList result;
  for ( AddrSpecList::const_iterator it = l.begin(), end = l.end() ; it != end ; ++it ) {
    const QString s = (*it).asString();
    if ( allowEmpty || !s.isEmpty() )
      result.push_back( s );
  }
  return result;
}


static void extractSenderToCCAndBcc( const KMime::Message::Ptr &aMsg, QString &sender, QStringList &to, QStringList &cc, QStringList &bcc )
{
  sender = aMsg->sender()->asUnicodeString();
  if( aMsg->headerByType("X-KMail-Recipients") ) {
    // extended BCC handling to prevent TOs and CCs from seeing
    // BBC information by looking at source of an OpenPGP encrypted mail
    to = addrSpecListToStringList( MessageHelper::extractAddrSpecs( aMsg, "X-KMail-Recipients" ) );
    aMsg->removeHeader( "X-KMail-Recipients" );
  } else {
    to = addrSpecListToStringList( MessageHelper::extractAddrSpecs( aMsg, "To" ) );
    cc = addrSpecListToStringList( MessageHelper::extractAddrSpecs( aMsg, "Cc" ) );
    bcc = addrSpecListToStringList( MessageHelper::extractAddrSpecs( aMsg, "Bcc" ) );
  }
}



AkonadiSender::AkonadiSender( QObject *parent )
  : QObject( parent )
{
}

bool AkonadiSender::doSend( const KMime::Message::Ptr &aMsg, short sendNow  )
{
  if( sendNow == -1 ) {
    sendNow = MessageComposer::MessageComposerSettings::self()->sendImmediate(); // -1 == use default setting
  }
  if ( !sendNow ) {
    sendOrQueueMessage( aMsg, MessageSender::SendLater );
  } else {
    sendOrQueueMessage( aMsg, MessageSender::SendImmediate );
  }
  return true;
}

bool AkonadiSender::doSendQueued( const QString &customTransport )
{
  kDebug() << "Sending queued message with custom transport:" << customTransport;
  mCustomTransport = customTransport;

  DispatcherInterface *dispatcher = new DispatcherInterface();
  if( mCustomTransport.isEmpty() ) {
    dispatcher->dispatchManually();
  } else {
    dispatcher->dispatchManualTransport( TransportManager::self()->transportByName( mCustomTransport )->id() );
  }
  delete dispatcher;
  return true;
}

void AkonadiSender::sendOrQueueMessage( const KMime::Message::Ptr &message, MessageSender::SendMethod method )
{
  Q_ASSERT( message );
  kDebug() << "KMime::Message: \n[\n" << message->encodedContent().left( 1000 ) << "\n]\n";

  MessageQueueJob *qjob = new MessageQueueJob( this );
  if ( message->headerByType( "X-KMail-Fcc" ) ) {
    qjob->sentBehaviourAttribute().setSentBehaviour(
                                 SentBehaviourAttribute::MoveToCollection );
    const int sentCollectionId = message->headerByType( "X-KMail-Fcc" )->asUnicodeString().toInt();
    qjob->sentBehaviourAttribute().setMoveToCollection(
                                 Akonadi::Collection( sentCollectionId ) );
  } else {
    qjob->sentBehaviourAttribute().setSentBehaviour(
           MailTransport::SentBehaviourAttribute::MoveToDefaultSentCollection );
  }
  qjob->setMessage( message );

  // Get transport.
  int transportId = -1;
  if ( !mCustomTransport.isEmpty() ) {
    transportId = TransportManager::self()->transportByName( mCustomTransport, true )->id();
  } else {
    transportId = message->headerByType( "X-KMail-Transport"  ) ? message->headerByType( "X-KMail-Transport" )->asUnicodeString().toInt() : -1;
  }
  const Transport *transport = TransportManager::self()->transportById( transportId );
  if( !transport ) { 
      kDebug()<<" No transport defined. Need to create it";
      return;
  }
  kDebug() << "Using transport (" << transport->name() << "," << transport->id() << ")";
  qjob->transportAttribute().setTransportId( transport->id() );

  // if we want to manually queue it for sending later, then do it
  if( method == MessageSender::SendLater )
    qjob->dispatchModeAttribute().setDispatchMode( MailTransport::DispatchModeAttribute::Manual );

  // Get addresses.
  QStringList to, cc, bcc;
  QString from;
  extractSenderToCCAndBcc( message, from, to, cc, bcc );
  qjob->addressAttribute().setFrom( from );
  qjob->addressAttribute().setTo( to );
  qjob->addressAttribute().setCc( cc );
  qjob->addressAttribute().setBcc( bcc );

  QList<Akonadi::Item::Id> originalMessageId;
  QList<Akonadi::MessageStatus> linkStatus;
  if ( MessageCore::Util::getLinkInformation( message, originalMessageId, linkStatus ) ) {
    Q_FOREACH( Akonadi::Item::Id id, originalMessageId )
    {
      if ( linkStatus.first() == Akonadi::MessageStatus::statusReplied() ) {
        qjob->sentActionAttribute().addAction( MailTransport::SentActionAttribute::Action::MarkAsReplied, QVariant( id ) );
      } else if ( linkStatus.first() == Akonadi::MessageStatus::statusForwarded() ) {
        qjob->sentActionAttribute().addAction( MailTransport::SentActionAttribute::Action::MarkAsForwarded, QVariant( id ) );
      }
    }
  }

  
  MessageCore::StringUtil::removePrivateHeaderFields( message );
  message->assemble();

  // Queue the message.
  connect( qjob, SIGNAL(result(KJob*)), this, SLOT(queueJobResult(KJob*)) );
  mPendingJobs.insert( qjob );
  qjob->start();
  kDebug() << "QueueJob started.";

  // TODO potential problem:
  // The MDA finishes sending a message before I queue the next one, and
  // thinking it is finished, the progress item deletes itself.
  // Turn the MDA offline until everything is queued?
}

void AkonadiSender::queueJobResult( KJob *job )
{
  Q_ASSERT( mPendingJobs.contains( job ) );
  mPendingJobs.remove( job );

  if( job->error() ) {
    kDebug() << "QueueJob failed with error" << job->errorString();
  } else {
    kDebug() << "QueueJob success.";
  }
}

#include "akonadisender.moc"
