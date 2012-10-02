/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
  Copyright (C) 2009 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>

  Parts based on KMail code by:

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

#include "util.h"

#include <QTextCodec>

#include <KCharsets>
#include <KDebug>
#include <KLocalizedString>
#include <KMessageBox>

#include <kmime/kmime_charfreq.h>
#include <kmime/kmime_content.h>
#include <kmime/kmime_util.h>
#include <mailtransport/messagequeuejob.h>
#include <akonadi/item.h>
#include <akonadi/kmime/messagestatus.h>
#include <akonadi/agentinstance.h>
#include <akonadi/agentmanager.h>
#include <messagecore/messagehelpers.h>

KMime::Content* Message::Util::composeHeadersAndBody( KMime::Content* orig, QByteArray encodedBody, Kleo::CryptoMessageFormat format, bool sign, QByteArray hashAlgo )
{

  KMime::Content* result = new KMime::Content;

  if( !( format & Kleo::InlineOpenPGPFormat ) ) { // make a MIME message
    // make headers and CE+CTE
    kDebug() << "making MIME message, format:" << format;
    makeToplevelContentType( result, format, sign, hashAlgo );
    const QByteArray boundary = KMime::multiPartBoundary();

    if( makeMultiMime( format, sign ) ) {
      result->contentType()->setBoundary( boundary );
    }

    if( format & Kleo::SMIMEOpaqueFormat ) {
      result->contentDisposition()->setDisposition( KMime::Headers::CDattachment );
      result->contentDisposition()->setFilename( QString::fromAscii( "smime.p7m" ) );
    }

    if( !makeMultiMime( format, sign ) && format & Kleo::AnySMIME ) {
      result->contentTransferEncoding()->setEncoding( KMime::Headers::CEbase64 );
    } else {
      result->contentTransferEncoding()->setEncoding( orig->contentTransferEncoding()->encoding() );

    }

    result->assemble();
    kDebug() << "processed header:" << result->head();
    // now make the body
    if( !makeMultiMime( format, sign ) ) {
      // set body to be body + encoded if there is a orig body
      // if not, ignore it because the newline messes up decrypting
      if(  sign && makeMultiPartSigned( format ) ) {
        result->setBody( orig->body() + "\n" + encodedBody );
      } else {
        result->setBody( encodedBody );
      }
    } else {
      // Build the encapsulated MIME parts.
      // Build a MIME part holding the version information
      // taking the body contents returned in
      // structuring.data.bodyTextVersion.
      KMime::Content* vers = 0;
      if( !sign && format == Kleo::OpenPGPMIMEFormat ) {
        vers = new KMime::Content;
        vers->contentType()->setMimeType( "application/pgp-encrypted" );
        vers->contentDisposition()->setDisposition( KMime::Headers::CDattachment );
        vers->setBody( "Version: 1" );

      }

      // Build a MIME part holding the code information
      // taking the body contents returned in ciphertext.
      KMime::Content* code = new KMime::Content;
      setNestedContentType( code, format, sign );
      setNestedContentDisposition( code, format, sign );

      if( format & Kleo::AnySMIME ) {
        code->contentTransferEncoding()->setEncoding( KMime::Headers::CEbase64 );
        code->contentTransferEncoding()->needToEncode();
      }
      code->setBody( encodedBody );

      // compose the multi-part message
      if( sign && makeMultiMime( format, true ) )
        result->addContent( orig );
      if( vers )
        result->addContent( vers );
      result->addContent( code );
    }
  } else { // not MIME, just plain message
    result->setHead( orig->head() );

    QByteArray resultingBody;
    if( sign && makeMultiMime( format, true ) )
      resultingBody += orig->body();
    if( !encodedBody.isEmpty() )
      resultingBody += encodedBody;
    else {
      kDebug() << "Got no encoded payload trying to save as plaintext inline pgp!";
    }

    result->setBody( resultingBody );
    result->parse();
  }
  return result;

}

// set the correct top-level ContentType on the message
void Message::Util::makeToplevelContentType( KMime::Content* content, Kleo::CryptoMessageFormat format, bool sign, QByteArray hashAlgo )
{
  switch ( format ) {
    default:
    case Kleo::InlineOpenPGPFormat:
    case Kleo::OpenPGPMIMEFormat:
      if( sign ) {
        content->contentType()->setMimeType( QByteArray( "multipart/signed" ) );
        content->contentType()->setParameter( QString::fromAscii( "protocol" ), QString::fromAscii( "application/pgp-signature" ) );
        content->contentType()->setParameter( QString::fromAscii( "micalg" ), QString::fromAscii( "pgp-" + hashAlgo ).toLower() );

      } else {
        content->contentType()->setMimeType( QByteArray( "multipart/encrypted" ) );
        content->contentType()->setParameter( QString::fromAscii( "protocol" ), QString::fromAscii( "application/pgp-encrypted" ) );
      }
      return;
    case Kleo::SMIMEFormat:
      if ( sign ) {
        kDebug() << "setting headers for SMIME";
        content->contentType()->setMimeType( QByteArray( "multipart/signed" ) );
        content->contentType()->setParameter( QString::fromAscii( "protocol" ), QString::fromAscii( "application/pkcs7-signature" ) );
        content->contentType()->setParameter( QString::fromAscii( "micalg" ), QString::fromAscii( hashAlgo ).toLower() );
        return;
      }
      // fall through (for encryption, there's no difference between
      // SMIME and SMIMEOpaque, since there is no mp/encrypted for
      // S/MIME)
    case Kleo::SMIMEOpaqueFormat:

      kDebug() << "setting headers for SMIME/opaque";
      content->contentType()->setMimeType( QByteArray( "application/pkcs7-mime" ) );

      if( sign ) {
        content->contentType()->setParameter( QString::fromAscii( "smime-type" ), QString::fromAscii( "signed-data" ) );
      } else {
        content->contentType()->setParameter( QString::fromAscii( "smime-type" ), QString::fromAscii( "enveloped-data" ) );
      }
      content->contentType()->setParameter( QString::fromAscii( "name" ), QString::fromAscii( "smime.p7m" ) );
  }
}


void Message::Util::setNestedContentType( KMime::Content* content, Kleo::CryptoMessageFormat format, bool sign )
{
  switch( format ){
    case Kleo::OpenPGPMIMEFormat:
      if( sign ) {
        content->contentType()->setMimeType( QByteArray( "application/pgp-signature" ) );
        content->contentType()->setParameter( QString::fromAscii( "name" ), QString::fromAscii( "signature.asc" ) );
        content->contentDescription()->from7BitString( "This is a digitally signed message part." );
      } else {
        content->contentType()->setMimeType( QByteArray( "application/octet-stream" ) );
      }
      return;
    case Kleo::SMIMEFormat:
      if ( sign ) {
        content->contentType()->setMimeType( QByteArray( "application/pkcs7-signature" ) );
        content->contentType()->setParameter( QString::fromAscii( "name" ), QString::fromAscii( "smime.p7s" ) );
        return;
      }
      // fall through:
    default:
    case Kleo::InlineOpenPGPFormat:
    case Kleo::SMIMEOpaqueFormat:
      ;
  }
}


void Message::Util::setNestedContentDisposition( KMime::Content* content, Kleo::CryptoMessageFormat format, bool sign )
{
  if ( !sign && format & Kleo::OpenPGPMIMEFormat ) {
    content->contentDisposition()->setDisposition( KMime::Headers::CDinline );
    content->contentDisposition()->setFilename( QString::fromAscii( "msg.asc" ) );
  } else if ( sign && format & Kleo::SMIMEFormat ) {
    content->contentDisposition()->setDisposition( KMime::Headers::CDattachment );
    content->contentDisposition()->setFilename( QString::fromAscii( "smime.p7s" ) );
  }
}


bool Message::Util::makeMultiMime( Kleo::CryptoMessageFormat format, bool sign )
{
  switch ( format ) {
    default:
    case Kleo::InlineOpenPGPFormat:
    case Kleo::SMIMEOpaqueFormat:   return false;
    case Kleo::OpenPGPMIMEFormat:   return true;
    case Kleo::SMIMEFormat:         return sign; // only on sign - there's no mp/encrypted for S/MIME
  }
}

bool Message::Util::makeMultiPartSigned( Kleo::CryptoMessageFormat f )
{
  return makeMultiMime( f, true );
}

QByteArray Message::Util::selectCharset( const QList<QByteArray> &charsets, const QString &text )
{
  foreach( const QByteArray &name, charsets ) {
    // We use KCharsets::codecForName() instead of QTextCodec::codecForName() here, because
    // the former knows us-ascii is latin1.
    QTextCodec *codec = KGlobal::charsets()->codecForName( QString::fromLatin1( name ) );
    if( !codec ) {
      kWarning() << "Could not get text codec for charset" << name;
      continue;
    }
    if( codec->canEncode( text ) ) {
      // Special check for us-ascii (needed because us-ascii is not exactly latin1).
      if( name == "us-ascii" && !KMime::isUsAscii( text ) ) {
        continue;
      }
      kDebug() << "Chosen charset" << name;
      return name;
    }
  }
  kDebug() << "No appropriate charset found.";
  return QByteArray();
}

QStringList Message::Util::AttachmentKeywords()
{
  return i18nc(
    "comma-separated list of keywords that are used to detect whether "
    "the user forgot to attach his attachment. Do not add space between words.",
    "attachment,attached" ).split( QLatin1Char( ',' ) );
}

QString Message::Util::cleanedUpHeaderString( const QString &s )
{
  // remove invalid characters from the header strings
  QString res( s );
  res.remove( QChar::fromLatin1( '\r' ) );
  res.replace( QChar::fromLatin1( '\n' ), QString::fromLatin1( " " ) );
  return res.trimmed();
}

void Message::Util::addSendReplyForwardAction(const KMime::Message::Ptr &message, MailTransport::MessageQueueJob *qjob)
{
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
}

bool Message::Util::sendMailDispatcherIsOnline( QWidget *parent )
{
  Akonadi::AgentInstance instance = Akonadi::AgentManager::self()->instance( QLatin1String( "akonadi_maildispatcher_agent" ) );
  if( !instance.isValid() ) {
    return false;
  }
  if ( instance.isOnline() )
    return true;
  else {
    const int rc = KMessageBox::warningYesNo( parent,i18n("The mail dispatcher is offline, so mails cannot be sent. Do you want to make it online?"),
                                        i18n("Mail dispatcher offline."), KStandardGuiItem::yes(), KStandardGuiItem::no(), QLatin1String("maildispatcher_put_online"));
    if ( rc == KMessageBox::No )
      return false;
    instance.setIsOnline( true );
    return true;
  } 
  return false;
}
  
