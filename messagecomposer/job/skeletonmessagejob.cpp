/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "settings/messagecomposersettings.h"
#include "skeletonmessagejob.h"
#include "kdepim-version.h"

#include "part/infopart.h"
#include "part/globalpart.h"
#include "jobbase_p.h"

#include <QTextCodec>
#include <QTimer>
#include <QHostInfo>
#include <QUrl>

#include <KCharsets>
#include <QDebug>
#include <KGlobal>
#include <KProtocolManager>

#include <KMime/kmime_message.h>
#include <KPIMUtils/email.h>

using namespace MessageComposer;

class MessageComposer::SkeletonMessageJobPrivate : public JobBasePrivate
{
public:
    SkeletonMessageJobPrivate( SkeletonMessageJob *qq )
        : JobBasePrivate( qq )
        , infoPart( 0 )
        , globalPart( 0 )
        , message( 0 )
    {
    }

    void doStart(); // slot

    InfoPart *infoPart;
    GlobalPart* globalPart;
    KMime::Message *message;

    Q_DECLARE_PUBLIC( SkeletonMessageJob )
};

void SkeletonMessageJobPrivate::doStart()
{
    Q_Q( SkeletonMessageJob );

    Q_ASSERT( infoPart );
    Q_ASSERT( message == 0 );
    message = new KMime::Message;

    // From:
    {
        KMime::Headers::From *from = new KMime::Headers::From( message );
        KMime::Types::Mailbox address;
        address.fromUnicodeString( KPIMUtils::normalizeAddressesAndEncodeIdn( infoPart->from() ) );
        from->addAddress( address );
        message->setHeader( from );
    }

    // To:
    {
        KMime::Headers::To *to = new KMime::Headers::To( message );
        foreach( const QString &a, infoPart->to() ) {
            KMime::Types::Mailbox address;
            address.fromUnicodeString( KPIMUtils::normalizeAddressesAndEncodeIdn( a ) );
            to->addAddress( address );
        }
        message->setHeader( to );
    }

    // Reply To:
    if( !infoPart->replyTo().isEmpty() )
    {
        KMime::Headers::ReplyTo *replyTo = new KMime::Headers::ReplyTo( message );
        KMime::Types::Mailbox address;
        address.fromUnicodeString( KPIMUtils::normalizeAddressesAndEncodeIdn( infoPart->replyTo() ) );
        replyTo->addAddress( address );
        message->setHeader( replyTo );
    }

    // Cc:
    {
        KMime::Headers::Cc *cc = new KMime::Headers::Cc( message );
        foreach( const QString &a, infoPart->cc() ) {
            KMime::Types::Mailbox address;
            address.fromUnicodeString( KPIMUtils::normalizeAddressesAndEncodeIdn( a ) );
            cc->addAddress( address );
        }
        message->setHeader( cc );
    }

    // Bcc:
    {
        KMime::Headers::Bcc *bcc = new KMime::Headers::Bcc( message );
        foreach( const QString &a, infoPart->bcc() ) {
            KMime::Types::Mailbox address;
            address.fromUnicodeString( KPIMUtils::normalizeAddressesAndEncodeIdn( a ) );
            bcc->addAddress( address );
        }
        message->setHeader( bcc );
    }

    // Subject:
    {
        KMime::Headers::Subject *subject = new KMime::Headers::Subject( message );
        subject->fromUnicodeString( infoPart->subject(), "utf-8" );
        // TODO should we be more specific about the charset?
        message->setHeader( subject );
    }

    // Date:
    {
        KMime::Headers::Date *date = new KMime::Headers::Date( message );
        date->setDateTime( KDateTime::currentLocalDateTime() );
        message->setHeader( date );
    }

    // Message-ID
    {
        KMime::Headers::MessageID *messageId = new KMime::Headers::MessageID();
        QByteArray fqdn;
        if( MessageComposer::MessageComposerSettings::self()->useCustomMessageIdSuffix() ) {
            fqdn = QUrl::toAce( MessageComposer::MessageComposerSettings::self()->customMsgIDSuffix() );
        }
        if( fqdn.isEmpty() ) {
            fqdn = QUrl::toAce( QHostInfo::localHostName() );
        }
        if( fqdn.isEmpty() ) {
            qWarning() << "Unable to generate a Message-ID, falling back to 'localhost.localdomain'.";
            fqdn = "local.domain";
        }
        messageId->generate( fqdn );
        message->setHeader( messageId );
    }
    // Extras

    foreach( KMime::Headers::Base* extra, infoPart->extraHeaders() ) {
        message->setHeader( extra );
    }

    // MDN
    {
        if( globalPart->MDNRequested() ) {
            const QString addr = infoPart->replyTo().isEmpty() ? infoPart->from() : infoPart->replyTo();
            KMime::Headers::Generic* mdn = new KMime::Headers::Generic( "Disposition-Notification-To", message, addr, "utf-8" );
            message->setHeader( mdn );
        }
    }

    // User-Agent
    if ( !infoPart->userAgent().isEmpty() ) {
        QStringList extraInfo;
        extraInfo << QLatin1String( KDEPIM_GIT_REVISION_STRING ) << QLatin1String( KDEPIM_GIT_LAST_CHANGE );
        KMime::Headers::UserAgent *ua = new KMime::Headers::UserAgent( message );
        ua->fromUnicodeString( KProtocolManager::userAgentForApplication( infoPart->userAgent(), QLatin1String( KDEPIM_VERSION ), extraInfo ), "utf-8" );
        message->setHeader( ua );
    }

    // Urgent header
    if ( infoPart->urgent() ) {
        KMime::Headers::Generic* urg1 = new KMime::Headers::Generic( "X-PRIORITY", message, QLatin1String( "2 (High)" ), "utf-8" );
        KMime::Headers::Generic* urg2 = new KMime::Headers::Generic( "Priority", message, QLatin1String( "urgent" ), "utf-8" );
        message->setHeader( urg1 );
        message->setHeader( urg2 );
    }

    // In-Reply-To
    if ( !infoPart->inReplyTo().isEmpty() ) {
        KMime::Headers::InReplyTo *header = new KMime::Headers::InReplyTo( message, infoPart->inReplyTo(), "utf-8" );
        message->setHeader( header );
    }

    // References
    if ( !infoPart->references().isEmpty() ) {
        KMime::Headers::References *header = new KMime::Headers::References( message, infoPart->references(), "utf-8" );
        message->setHeader( header );
    }

    q->emitResult(); // Success.
}


SkeletonMessageJob::SkeletonMessageJob( InfoPart *infoPart, GlobalPart* globalPart, QObject *parent )
    : JobBase( *new SkeletonMessageJobPrivate( this ), parent )
{
    Q_D( SkeletonMessageJob );
    d->infoPart = infoPart;
    d->globalPart = globalPart;
}

SkeletonMessageJob::~SkeletonMessageJob()
{
}

InfoPart *SkeletonMessageJob::infoPart() const
{
    Q_D( const SkeletonMessageJob );
    return d->infoPart;
}

void SkeletonMessageJob::setInfoPart( InfoPart *part )
{
    Q_D( SkeletonMessageJob );
    d->infoPart = part;
}


GlobalPart *SkeletonMessageJob::globalPart() const
{
    Q_D( const SkeletonMessageJob );
    return d->globalPart;
}

void SkeletonMessageJob::setGlobalPart( GlobalPart *part )
{
    Q_D( SkeletonMessageJob );
    d->globalPart = part;
}


KMime::Message *SkeletonMessageJob::message() const
{
    Q_D( const SkeletonMessageJob );
    return d->message;
}

void SkeletonMessageJob::start()
{
    Q_D( SkeletonMessageJob );
    d->doStart();
}

#include "moc_skeletonmessagejob.cpp"
