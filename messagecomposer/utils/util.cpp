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

#include "composer/composer.h"
#include "singlepartjob.h"

#include <QTextCodec>
#include <QTextEdit>

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
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/agentmanager.h>
#include <messagecore/helpers/messagehelpers.h>

KMime::Content* setBodyAndCTE( QByteArray &encodedBody, KMime::Headers::ContentType* contentType, KMime::Content* ret ){
  MessageComposer::Composer composer;
  MessageComposer::SinglepartJob cteJob( &composer );

  cteJob.contentType()->setMimeType( contentType->mimeType() );
  cteJob.contentType()->setCharset( contentType->charset() );
  cteJob.setData( encodedBody );
  cteJob.exec();
  cteJob.content()->assemble();

  ret->contentTransferEncoding()->setEncoding( cteJob.contentTransferEncoding()->encoding() );
  ret->setBody( cteJob.content()->encodedBody() );

  return ret;
}

KMime::Content* MessageComposer::Util::composeHeadersAndBody( KMime::Content* orig, QByteArray encodedBody, Kleo::CryptoMessageFormat format, bool sign, QByteArray hashAlgo )
{
  KMime::Content* result = new KMime::Content;

  // called should have tested that the signing/encryption failed
  Q_ASSERT( !encodedBody.isEmpty() );

  if( !( format & Kleo::InlineOpenPGPFormat ) ) { // make a MIME message
    kDebug() << "making MIME message, format:" << format;
    makeToplevelContentType( result, format, sign, hashAlgo );

    if( makeMultiMime( format, sign ) ) {   // sign/enc PGPMime, sign SMIME

      const QByteArray boundary = KMime::multiPartBoundary();
      result->contentType()->setBoundary( boundary );

      result->assemble();
      //kDebug() << "processed header:" << result->head();

      // Build the encapsulated MIME parts.
      // Build a MIME part holding the code information
      // taking the body contents returned in ciphertext.
      KMime::Content* code = new KMime::Content;
      setNestedContentType( code, format, sign );
      setNestedContentDisposition( code, format, sign );

      if ( sign ) {                         // sign PGPMime, sign SMIME
        if( format & Kleo::AnySMIME ) {     // sign SMIME
          code->contentTransferEncoding()->setEncoding( KMime::Headers::CEbase64 );
          code->contentTransferEncoding()->needToEncode();
          code->setBody( encodedBody );
        } else {                            // sign PGPMmime
          setBodyAndCTE( encodedBody, orig->contentType(), code );
        }
        result->addContent( orig );
        result->addContent( code );
      } else {                              // enc PGPMime
        setBodyAndCTE( encodedBody, orig->contentType(), code );

        // Build a MIME part holding the version information
        // taking the body contents returned in
        // structuring.data.bodyTextVersion.
        KMime::Content* vers = new KMime::Content;
        vers->contentType()->setMimeType( "application/pgp-encrypted" );
        vers->contentDisposition()->setDisposition( KMime::Headers::CDattachment );
        vers->contentTransferEncoding()->setEncoding( KMime::Headers::CE7Bit );
        vers->setBody( "Version: 1" );

        result->addContent( vers );
        result->addContent( code );
      }
    } else {                                //enc SMIME, sign/enc SMIMEOpaque
      result->contentTransferEncoding()->setEncoding( KMime::Headers::CEbase64 );
      result->contentDisposition()->setDisposition( KMime::Headers::CDattachment );
      result->contentDisposition()->setFilename( QString::fromLatin1( "smime.p7m" ) );

      result->assemble();
      //kDebug() << "processed header:" << result->head();

      result->setBody( encodedBody );
    }
  } else {                                  // sign/enc PGPInline
    result->setHead( orig->head() );
    result->parse();

    // fixing ContentTransferEncoding
    setBodyAndCTE( encodedBody, orig->contentType(), result );
  }
  return result;
}

// set the correct top-level ContentType on the message
void MessageComposer::Util::makeToplevelContentType( KMime::Content* content, Kleo::CryptoMessageFormat format, bool sign, QByteArray hashAlgo )
{
  switch ( format ) {
    default:
    case Kleo::InlineOpenPGPFormat:
    case Kleo::OpenPGPMIMEFormat:
      if( sign ) {
        content->contentType()->setMimeType( QByteArray( "multipart/signed" ) );
        content->contentType()->setParameter( QString::fromLatin1( "protocol" ), QString::fromAscii( "application/pgp-signature" ) );
        content->contentType()->setParameter( QString::fromLatin1( "micalg" ), QString::fromAscii( "pgp-" + hashAlgo ).toLower() );

      } else {
        content->contentType()->setMimeType( QByteArray( "multipart/encrypted" ) );
        content->contentType()->setParameter( QString::fromLatin1( "protocol" ), QString::fromAscii( "application/pgp-encrypted" ) );
      }
      return;
    case Kleo::SMIMEFormat:
      if ( sign ) {
        kDebug() << "setting headers for SMIME";
        content->contentType()->setMimeType( QByteArray( "multipart/signed" ) );
        content->contentType()->setParameter( QString::fromLatin1( "protocol" ), QString::fromAscii( "application/pkcs7-signature" ) );
        content->contentType()->setParameter( QString::fromLatin1( "micalg" ), QString::fromAscii( hashAlgo ).toLower() );
        return;
      }
      // fall through (for encryption, there's no difference between
      // SMIME and SMIMEOpaque, since there is no mp/encrypted for
      // S/MIME)
    case Kleo::SMIMEOpaqueFormat:

      kDebug() << "setting headers for SMIME/opaque";
      content->contentType()->setMimeType( QByteArray( "application/pkcs7-mime" ) );

      if( sign ) {
        content->contentType()->setParameter( QString::fromLatin1( "smime-type" ), QString::fromAscii( "signed-data" ) );
      } else {
        content->contentType()->setParameter( QString::fromLatin1( "smime-type" ), QString::fromAscii( "enveloped-data" ) );
      }
      content->contentType()->setParameter( QString::fromLatin1( "name" ), QString::fromAscii( "smime.p7m" ) );
  }
}


void MessageComposer::Util::setNestedContentType( KMime::Content* content, Kleo::CryptoMessageFormat format, bool sign )
{
  switch( format ){
    case Kleo::OpenPGPMIMEFormat:
      if( sign ) {
        content->contentType()->setMimeType( QByteArray( "application/pgp-signature" ) );
        content->contentType()->setParameter( QString::fromLatin1( "name" ), QString::fromAscii( "signature.asc" ) );
        content->contentDescription()->from7BitString( "This is a digitally signed message part." );
      } else {
        content->contentType()->setMimeType( QByteArray( "application/octet-stream" ) );
      }
      return;
    case Kleo::SMIMEFormat:
      if ( sign ) {
        content->contentType()->setMimeType( QByteArray( "application/pkcs7-signature" ) );
        content->contentType()->setParameter( QString::fromLatin1( "name" ), QString::fromAscii( "smime.p7s" ) );
        return;
      }
      // fall through:
    default:
    case Kleo::InlineOpenPGPFormat:
    case Kleo::SMIMEOpaqueFormat:
      ;
  }
}


void MessageComposer::Util::setNestedContentDisposition( KMime::Content* content, Kleo::CryptoMessageFormat format, bool sign )
{
  if ( !sign && format & Kleo::OpenPGPMIMEFormat ) {
    content->contentDisposition()->setDisposition( KMime::Headers::CDinline );
    content->contentDisposition()->setFilename( QString::fromLatin1( "msg.asc" ) );
  } else if ( sign && format & Kleo::SMIMEFormat ) {
    content->contentDisposition()->setDisposition( KMime::Headers::CDattachment );
    content->contentDisposition()->setFilename( QString::fromLatin1( "smime.p7s" ) );
  }
}


bool MessageComposer::Util::makeMultiMime( Kleo::CryptoMessageFormat format, bool sign )
{
  switch ( format ) {
    default:
    case Kleo::InlineOpenPGPFormat:
    case Kleo::SMIMEOpaqueFormat:   return false;
    case Kleo::OpenPGPMIMEFormat:   return true;
    case Kleo::SMIMEFormat:         return sign; // only on sign - there's no mp/encrypted for S/MIME
  }
}

bool MessageComposer::Util::makeMultiPartSigned( Kleo::CryptoMessageFormat f )
{
  return makeMultiMime( f, true );
}

QByteArray MessageComposer::Util::selectCharset( const QList<QByteArray> &charsets, const QString &text )
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

QStringList MessageComposer::Util::AttachmentKeywords()
{
  return i18nc(
    "comma-separated list of keywords that are used to detect whether "
    "the user forgot to attach his attachment. Do not add space between words.",
    "attachment,attached" ).split( QLatin1Char( ',' ) );
}

QString MessageComposer::Util::cleanedUpHeaderString( const QString &s )
{
  // remove invalid characters from the header strings
  QString res( s );
  res.remove( QChar::fromLatin1( '\r' ) );
  res.replace( QChar::fromLatin1( '\n' ), QString::fromLatin1( " " ) );
  return res.trimmed();
}

void MessageComposer::Util::addSendReplyForwardAction(const KMime::Message::Ptr &message, MailTransport::MessageQueueJob *qjob)
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

bool MessageComposer::Util::sendMailDispatcherIsOnline( QWidget *parent )
{
  Akonadi::AgentInstance instance = Akonadi::AgentManager::self()->instance( QLatin1String( "akonadi_maildispatcher_agent" ) );
  if( !instance.isValid() ) {
    const int rc = KMessageBox::warningYesNo(parent,i18n("The mail dispatcher is not set up, so mails cannot be sent. Do you want to create a mail dispatcher?"),
                                        i18n("No mail dispatcher."), KStandardGuiItem::yes(), KStandardGuiItem::no(), QLatin1String("no_maildispatcher"));
    if (rc == KMessageBox::Yes) {
        const Akonadi::AgentType type = Akonadi::AgentManager::self()->type(QLatin1String("akonadi_maildispatcher_agent"));
        Q_ASSERT(type.isValid());
        Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob(type); // async. We'll have to try again later.
        job->start();
    }
    return false;
  }
  if ( instance.isOnline() )
    return true;
  else {
    const int rc = KMessageBox::warningYesNo( parent,i18n("The mail dispatcher is offline, so mails cannot be sent. Do you want to make it online?"),
                                        i18n("Mail dispatcher offline."), KStandardGuiItem::yes(), KStandardGuiItem::no(), QLatin1String("maildispatcher_put_online"));
    if (rc == KMessageBox::Yes) {
        instance.setIsOnline( true );
        return true;
    }
  }
  return false;
}
  
QString MessageComposer::Util::rot13(const QString &s)
{
  QString r(s);

  const int rLenght(r.length());
  for (int i=0; i<rLenght; ++i) {
    if ( ( r[i] >= QLatin1Char('A') && r[i] <= QLatin1Char('M') ) ||
         ( r[i] >= QLatin1Char('a') && r[i] <= QLatin1Char('m') ) ) {
      r[i] = (char)((int)QChar(r[i]).toLatin1() + 13);
    } else {
      if  ( ( r[i] >= QLatin1Char('N') && r[i] <= QLatin1Char('Z') ) ||
            ( r[i] >= QLatin1Char('n') && r[i] <= QLatin1Char('z') ) ) {
        r[i] = (char)((int)QChar(r[i]).toLatin1() - 13);
      }
    }
  }

  return r;
}

void MessageComposer::Util::addTextBox(QTextEdit *edit)
{
  QTextCursor cursor = edit->textCursor();
  if ( cursor.hasSelection() )
  {
    QString s = cursor.selectedText();
    s.prepend( QLatin1String(",----[  ]\n| " ));
    s.replace( QChar::ParagraphSeparator,QLatin1String("\n| ") );
    s.append( QLatin1String("\n`----") );
    edit->insertPlainText( s );
  } else {
    //int oldPos = cursor.position();
    cursor.movePosition( QTextCursor::StartOfBlock );
    cursor.movePosition( QTextCursor::EndOfBlock, QTextCursor::KeepAnchor );
    const QString s = cursor.selectedText();
    const QString str = QString::fromLatin1(",----[  ]\n| %1\n`----").arg(s);
    cursor.insertText( str );
    //cursor.setPosition( qMax( 0, oldPos - 2 ) );
    edit->setTextCursor( cursor );
  }
}

