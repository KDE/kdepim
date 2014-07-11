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

#include <qdebug.h>


Composer::Composer( QObject *parent )
    : QObject( parent )
    , m_receiverModel ( new ReceiverModel (this) )
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

    // Set the headers.
    m_msg->from()->fromUnicodeString( m_from , "utf-8" );
    m_msg->to()->fromUnicodeString( m_receiverModel->recipientString(MessageComposer::Recipient::To), "utf-8" );
    m_msg->cc()->fromUnicodeString( m_receiverModel->recipientString(MessageComposer::Recipient::Cc), "utf-8" );
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
