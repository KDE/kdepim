/*
Copyright 2014  Abhijeet Nikam connect08nikam@gmail.com

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

#include "composer.h"

#include <AkonadiCore/ItemFetchScope>

Composer::Composer( QObject *parent )
    : QObject( parent )
    , m_receiverModel ( new ReceiverModel (this) )
    , m_IdentityManager ( new KIdentityManagement::IdentityManager( false, this ) )
{

}

QString Composer::from() const
{
    return m_from;
}

QString Composer::cc() const
{
    return m_cc;
}


QString Composer::bcc() const
{
    return m_bcc;
}


QString Composer::to() const
{
    return m_to;
}

QString Composer::subject() const
{
    return m_subject;
}

QString Composer::body() const
{
    return m_body;
}


void Composer::setFrom(const QString &from)
{
    if ( from != m_from ) {
        m_from = from;
        emit fromChanged();
    }

}


void Composer::setTo(const QString &to)
{
    if ( to != m_to ) {
        m_to = to;
        emit toChanged();
    }
}


void Composer::setCC(const QString &cc)
{
    if ( cc != m_cc ) {
        m_cc = cc;
        emit ccChanged();
    }
}


void Composer::setBCC(const QString &bcc)
{
    if ( bcc != m_bcc ) {
        m_bcc = bcc;
        emit bccChanged();
    }
}

void Composer::setSubject(const QString &subject)
{
    if ( subject != m_subject ) {
        m_subject = subject;
        emit subjectChanged();
    }
}

void Composer::setBody(const QString &body)
{
    if ( body != m_body ) {
        m_body = body;
        emit bodyChanged();
    }
}

ReceiverModel *Composer::receiverModel() const
{

    return m_receiverModel;

}


QByteArray Composer::convert (const QString &body)
{

    QTextCodec *codec = QTextCodec::codecForName("UTF-16");
    QTextEncoder *encoderWithoutBom = codec->makeEncoder( QTextCodec::IgnoreHeader );
    QByteArray bytes  = encoderWithoutBom ->fromUnicode( body );

    return bytes;

}

void Composer::send()
{

    KMime::Message::Ptr m_msg (new KMime::Message);
    KMime::Headers::ContentType *ct = m_msg->contentType();

    ct->setMimeType( "multipart/mixed" );
    ct->setBoundary( KMime::multiPartBoundary() );
    ct->setCategory( KMime::Headers::CCcontainer );
    m_msg->contentTransferEncoding()->clear();


    m_msg->from()->fromUnicodeString( m_from , "utf-8" );
    m_msg->to()->fromUnicodeString( m_receiverModel->recipientString(MessageComposer::Recipient::To), "utf-8" );
    m_msg->cc()->fromUnicodeString( m_receiverModel->recipientString(MessageComposer::Recipient::Cc), "utf-8" );
    m_msg->bcc()->fromUnicodeString( m_receiverModel->recipientString(MessageComposer::Recipient::Bcc), "utf-8" );
    m_msg->date()->setDateTime( QDateTime::currentDateTime() );
    m_msg->subject()->fromUnicodeString( m_subject, "utf-8" );

    // Set the first multipart, the body message.
    KMime::Content *b = new KMime::Content;
    b->contentType()->setMimeType( "text/plain" );
    b->setBody( convert (m_body) );

    // Add the multipart and assemble
    m_msg->addContent( b );
    m_msg->assemble();

    MessageComposer::AkonadiSender *mSender = new MessageComposer::AkonadiSender (this);
    mSender->send(m_msg, MessageComposer::MessageSender::SendImmediate);

}

void Composer::sendLater()
{

}

void Composer::saveDraft()
{

}


void Composer::addRecipient( const QString &email , int type )
{
    MessageComposer::Recipient::Ptr rec (new MessageComposer::Recipient);
    rec->setEmail ( email );
    rec->setType (  MessageComposer::Recipient::idToType(type) );
    m_receiverModel->addRecipient ( rec );
}


void Composer::replyToAll( const QUrl &url )
{

    reply ( url, MessageComposer::ReplyAll );

}

void Composer::replyToAuthor( const QUrl &url )
{

    reply ( url, MessageComposer::ReplyAuthor );

}

void Composer::replyToMailingList( const QUrl &url )
{

    reply ( url , MessageComposer::ReplyList );

}

void Composer::replyToMessage( const QUrl &url )
{

    reply ( url, MessageComposer::ReplySmart );

}

void Composer::forwardMessage(const QUrl &url)
{
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(Akonadi::Item::fromUrl(url));
    job->fetchScope().fetchFullPayload();

    connect( job, SIGNAL(result(KJob*)), SLOT(forwardFetchResult(KJob*)) );
}


void Composer::forwardFetchResult(KJob *job)
{
    const Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob*>( job );
    if ( job->error() || fetchJob->items().isEmpty() )
        return;

    const Akonadi::Item item = fetchJob->items().first();
    if ( !item.hasPayload<KMime::Message::Ptr>() )
        return;

    MessageComposer::MessageFactory factory( item.payload<KMime::Message::Ptr>(), item.id() );
    factory.setIdentityManager( m_IdentityManager );

    KMime::Message::Ptr fw =  factory.createForward();

    setMessage( fw );
}


void Composer::reply(const QUrl &url, MessageComposer::ReplyStrategy replyStrategy, bool quoteOriginal)
{
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(Akonadi::Item::fromUrl(url));
    job->fetchScope().fetchFullPayload();

    job->setProperty( "replyStrategy", QVariant::fromValue( replyStrategy ) );
    job->setProperty( "quoteOriginal", QVariant::fromValue( quoteOriginal ) );

    connect( job, SIGNAL(result(KJob*)), SLOT(replyFetchResult(KJob*)) );

}


void Composer::replyFetchResult(KJob *job)
{

    const Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob*>( job );
    if ( job->error() || fetchJob->items().isEmpty() )
        return;

    const Akonadi::Item item = fetchJob->items().first();
    if ( !item.hasPayload<KMime::Message::Ptr>() )
        return;


    MessageComposer::MessageFactory factory( item.payload<KMime::Message::Ptr>(), item.id() );
    factory.setReplyStrategy( fetchJob->property( "replyStrategy" ).value<MessageComposer::ReplyStrategy>() );
    factory.setIdentityManager( m_IdentityManager );
    factory.setQuote( fetchJob->property( "quoteOriginal" ).toBool() );

    MessageComposer::MessageFactory::MessageReply reply =  factory.createReply();

    setMessage( reply.msg );

}


void Composer::setMessage ( const KMime::Message::Ptr &message )
{

    m_subject = message->subject()->asUnicodeString();

    m_receiverModel->setRecipientString( message->to()->mailboxes(), MessageComposer::Recipient::To );
    m_receiverModel->setRecipientString( message->cc()->mailboxes(), MessageComposer::Recipient::Cc );
    m_receiverModel->setRecipientString( message->bcc()->mailboxes(), MessageComposer::Recipient::Bcc );

    m_body = QLatin1String(message->body());

}

