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

#include "maintextjob.h"

#include "contentjobbase_p.h"
#include "part/globalpart.h"
#include "multipartjob.h"
#include "singlepartjob.h"
#include "part/textpart.h"
#include "util.h"

#include <QTextCodec>

#include <KCharsets>
#include <KDebug>
#include <KGlobal>
#include <KLocalizedString>
#include <KMessageBox>

#include <kmime/kmime_content.h>

#include <kpimtextedit/textedit.h>

using namespace MessageComposer;

class MessageComposer::MainTextJobPrivate : public ContentJobBasePrivate
{
  public:
    MainTextJobPrivate( MainTextJob *qq )
      : ContentJobBasePrivate( qq )
      , textPart( 0 )
    {
    }

    bool chooseSourcePlainText();
    bool chooseCharsetAndEncode();
    bool chooseCharset();
    bool encodeTexts();
    SinglepartJob *createPlainTextJob();
    SinglepartJob *createHtmlJob();
    SinglepartJob *createImageJob( const QSharedPointer<KPIMTextEdit::EmbeddedImage> &image );

    TextPart *textPart;
    QByteArray chosenCharset;
    QString sourcePlainText;
    QByteArray encodedPlainText;
    QByteArray encodedHtml;

    Q_DECLARE_PUBLIC( MainTextJob )
};

bool MainTextJobPrivate::chooseSourcePlainText()
{
  Q_Q( MainTextJob );
  Q_ASSERT( textPart );
  if( textPart->isWordWrappingEnabled() ) {
    sourcePlainText = textPart->wrappedPlainText();
    if( sourcePlainText.isEmpty() &&
        !textPart->cleanPlainText().isEmpty() ) {
      q->setError( JobBase::BugError );
      q->setErrorText( i18n( "Asked to use word wrapping, but not given wrapped plain text." ) );
      return false;
    }
  } else {
    sourcePlainText = textPart->cleanPlainText();
    if( sourcePlainText.isEmpty() &&
        !textPart->wrappedPlainText().isEmpty() ) {
      q->setError( JobBase::BugError );
      q->setErrorText( i18n( "Asked not to use word wrapping, but not given clean plain text." ) );
      return false;
    }
  }
  return true;
}

bool MainTextJobPrivate::chooseCharsetAndEncode()
{
  Q_Q( MainTextJob );

  const QList<QByteArray> charsets = q->globalPart()->charsets( true );
  if( charsets.isEmpty() ) {
    q->setError( JobBase::BugError );
    q->setErrorText( i18n( "No charsets were available for encoding. Please check your configuration and make sure it contains at least one charset for sending." ) );
    return false;
  }

  Q_ASSERT( textPart );
  QString toTry = sourcePlainText;
  if( textPart->isHtmlUsed() ) {
    toTry = textPart->cleanHtml();
  }
  chosenCharset = MessageComposer::Util::selectCharset( charsets, toTry );
  if( !chosenCharset.isEmpty() ) {
    // Good, found a charset that encodes the data without loss.
    return encodeTexts();
  } else {
    // No good charset was found.
    if( q->globalPart()->isGuiEnabled() && textPart->warnBadCharset() ) {
      // Warn the user and give them a chance to go back.
      int result = KMessageBox::warningYesNo(
          q->globalPart()->parentWidgetForGui(),
          i18n( "Encoding the message with %1 will lose some characters.\n"
                "Do you want to continue?", QString::fromLatin1( charsets.first() ) ),
          i18n( "Some Characters Will Be Lost" ),
          KGuiItem( i18n("Lose Characters") ),
          KGuiItem( i18n("Change Encoding") )
        );
      if( result == KMessageBox::No ) {
        q->setError( JobBase::UserCancelledError );
        q->setErrorText( i18n( "User decided to change the encoding." ) );
        return false;
      } else {
        chosenCharset = charsets.first();
        return encodeTexts();
      }
    } else if( textPart->warnBadCharset() ) {
      // Should warn user but no Gui available.
      kDebug() << "warnBadCharset but Gui is disabled.";
      q->setError( JobBase::UserError );
      q->setErrorText( i18n( "The selected encoding (%1) cannot fully encode the message.",
                             QString::fromLatin1( charsets.first() ) ) );
      return false;
    } else {
      // OK to go ahead with a bad charset.
      chosenCharset = charsets.first();
      return encodeTexts();

      // FIXME: This is based on the assumption that QTextCodec will replace
      // unknown characters with '?' or some other meaningful thing.  The code in
      // QTextCodec indeed uses '?', but this behaviour is not documented.
    }
  }

  // Should not reach here.
  Q_ASSERT( false );
  return false;
}

bool MainTextJobPrivate::encodeTexts()
{
  Q_Q( MainTextJob );
  QTextCodec *codec = KGlobal::charsets()->codecForName( QString::fromLatin1( chosenCharset ) );
  if( !codec ) {
    kError() << "Could not get text codec for charset" << chosenCharset;
    q->setError( JobBase::BugError );
    q->setErrorText( i18n( "Could not get text codec for charset \"%1\".", QString::fromLatin1( chosenCharset ) ) );
    return false;
  }
  encodedPlainText = codec->fromUnicode( sourcePlainText );
  if( !textPart->cleanHtml().isEmpty() )
    encodedHtml = codec->fromUnicode( textPart->cleanHtml() );
  kDebug() << "Done.";
  return true;
}

SinglepartJob *MainTextJobPrivate::createPlainTextJob()
{
  SinglepartJob *cjob = new SinglepartJob; // No parent.
  cjob->contentType()->setMimeType( "text/plain" );
  cjob->contentType()->setCharset( chosenCharset );
  cjob->setData( encodedPlainText );
  // TODO standard recommends Content-ID.
  return cjob;
}

SinglepartJob *MainTextJobPrivate::createHtmlJob()
{
  SinglepartJob *cjob = new SinglepartJob; // No parent.
  cjob->contentType()->setMimeType( "text/html" );
  cjob->contentType()->setCharset( chosenCharset );
  QByteArray data = KPIMTextEdit::TextEdit::imageNamesToContentIds( encodedHtml,
      textPart->embeddedImages() );
  cjob->setData( data );
  // TODO standard recommends Content-ID.
  return cjob;
}

SinglepartJob *MainTextJobPrivate::createImageJob( const QSharedPointer<KPIMTextEdit::EmbeddedImage> &image )
{
  Q_Q( MainTextJob );

  // The image is a PNG encoded with base64.
  SinglepartJob *cjob = new SinglepartJob; // No parent.
  cjob->contentType()->setMimeType( "image/png" );
  const QByteArray charset = MessageComposer::Util::selectCharset( q->globalPart()->charsets( true ), image->imageName );
  Q_ASSERT( !charset.isEmpty() );
  cjob->contentType()->setName( image->imageName, charset );
  cjob->contentTransferEncoding()->setEncoding( KMime::Headers::CEbase64 );
  cjob->contentTransferEncoding()->setDecoded( false ); // It is already encoded.
  cjob->contentID()->setIdentifier( image->contentID.toLatin1() );
  kDebug() << "cid" << cjob->contentID()->identifier();
  cjob->setData( image->image );
  return cjob;
}

MainTextJob::MainTextJob( TextPart *textPart, QObject *parent )
  : ContentJobBase( *new MainTextJobPrivate( this ), parent )
{
  Q_D( MainTextJob );
  d->textPart = textPart;
}

MainTextJob::~MainTextJob()
{
}

TextPart *MainTextJob::textPart() const
{
  Q_D( const MainTextJob );
  return d->textPart;
}

void MainTextJob::setTextPart( TextPart *part )
{
  Q_D( MainTextJob );
  d->textPart = part;
}

void MainTextJob::doStart()
{
  Q_D( MainTextJob );
  Q_ASSERT( d->textPart );

  // Word wrapping.
  if( !d->chooseSourcePlainText() ) {
    // chooseSourcePlainText has set an error.
    Q_ASSERT( error() );
    emitResult();
    return;
  }

  // Charset.
  if( !d->chooseCharsetAndEncode() ) {
    // chooseCharsetAndEncode has set an error.
    Q_ASSERT( error() );
    emitResult();
    return;
  }

  // Assemble the Content.
  SinglepartJob *plainJob = d->createPlainTextJob();
  if( d->encodedHtml.isEmpty() ) {
    kDebug() << "Making text/plain";
    // Content is text/plain.
    appendSubjob( plainJob );
  } else {
    MultipartJob *alternativeJob = new MultipartJob;
    alternativeJob->setMultipartSubtype( "alternative" );
    alternativeJob->appendSubjob( plainJob ); // text/plain first.
    alternativeJob->appendSubjob( d->createHtmlJob() ); // text/html second.
    if( !d->textPart->hasEmbeddedImages() ) {
      kDebug() << "Have no images.  Making multipart/alternative.";
      // Content is multipart/alternative.
      appendSubjob( alternativeJob );
    } else {
      kDebug() << "Have related images.  Making multipart/related.";
      // Content is multipart/related with a multipart/alternative sub-Content.
      MultipartJob *multipartJob = new MultipartJob;
      multipartJob->setMultipartSubtype( "related" );
      multipartJob->appendSubjob( alternativeJob );
      foreach( const QSharedPointer<KPIMTextEdit::EmbeddedImage> &image, d->textPart->embeddedImages() ) {
        multipartJob->appendSubjob( d->createImageJob( image ) );
      }
      appendSubjob( multipartJob );
    }
  }
  ContentJobBase::doStart();
}

void MainTextJob::process()
{
  Q_D( MainTextJob );
  // The content has been created by our subjob.
  Q_ASSERT( d->subjobContents.count() == 1 );
  d->resultContent = d->subjobContents.first();
  emitResult();
}

