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

#include "composer.h"
#include "contentjob.h"
#include "job_p.h"
#include "textpart.h"

#include <QTextCodec>

#include <KCharsets>
#include <KDebug>
#include <KGlobal>
#include <KLocalizedString>
#include <KMessageBox>

#include <kmime/kmime_content.h>
#include <kmime/kmime_util.h>

using namespace MessageComposer;
using namespace KMime;

class MessageComposer::MainTextJobPrivate : public JobPrivate
{
  public:
    MainTextJobPrivate( MainTextJob *qq )
      : JobPrivate( qq )
      , textPart( 0 )
    {
    }

    bool chooseCharsetAndEncode();
    bool chooseCharset();
    void encodeTexts();

    TextPart *textPart;
    QList<QByteArray> charsets;
    QByteArray chosenCharset;
    QString sourcePlainText;
    QByteArray encodedPlainText;
    QByteArray encodedHtml;

    // TODO related images

    Q_DECLARE_PUBLIC( MainTextJob )
};

bool MainTextJobPrivate::chooseCharsetAndEncode()
{
  Q_Q( MainTextJob );
  Q_ASSERT( composer );
  const Behaviour &beh = composer->behaviour();

  Q_ASSERT( textPart );
  charsets = textPart->charsets();
  foreach( const QByteArray &name, textPart->charsets() ) {
    charsets << name.toLower();
  }
  if( beh.isActionEnabled( Behaviour::UseFallbackCharset ) ) {
    charsets << "utf-8";
    // TODO somehow save the chosen charset in a custom header if behaviour allows it...
  }
  if( charsets.isEmpty() ) {
    q->setError( Job::BugError );
    q->setErrorText( i18n( "No charsets were available for encoding,"
                           " and the fallback charset was disabled." ) );
    return false;
  }

  if( chooseCharset() ) {
    // Good, one of the charsets can encode the data without loss.
    encodeTexts();
    return true;
  } else {
    // No good charset was found.
    if( beh.isActionEnabled( Behaviour::UseGui ) &&
        beh.isActionEnabled( Behaviour::WarnBadCharset ) ) {
      // Warn the user and give them a chance to go back.
      int result = KMessageBox::warningYesNo(
          composer->parentWidget(),
          i18n( "Encoding the message with %1 will lose some characters.\n"
                "Do you want to continue?", QString::fromLatin1( charsets.first() ) ),
          i18n( "Some Characters Will Be Lost" ),
          KGuiItem( i18n("Lose Characters") ),
          KGuiItem( i18n("Change Encoding") )
        );
      if( result == KMessageBox::No ) {
        q->setError( Job::UserCancelledError );
        q->setErrorText( i18n( "User decided to change the encoding." ) );
        return false;
      } else {
        chosenCharset = charsets.first();
        encodeTexts();
        return true;
      }
    } else if( beh.isActionEnabled( Behaviour::WarnBadCharset ) ) {
      // Should warn user but no Gui available.
      kDebug() << "WarnBadCharset but not UseGui.";
      q->setError( Job::UserError );
      q->setErrorText( i18n( "The selected encoding (%1) cannot fully encode the message.",
                             QString::fromLatin1( charsets.first() ) ) );
      return false;
    } else {
      // OK to go ahead with a bad charset.
      chosenCharset = charsets.first();
      encodeTexts();
      return true;

      // FIXME: This is based on the assumption that QTextCodec will replace
      // unknown characters with '?' or some other meaningful thing.  The code in
      // QTextCodec indeed uses '?', but this behaviour is not documented.
    }
  }

  // Should not reach here.
  Q_ASSERT( false );
  return false;
}

bool MainTextJobPrivate::chooseCharset()
{
  Q_ASSERT( !charsets.isEmpty() );
  Q_ASSERT( textPart );
  QString toTry = sourcePlainText;
  if( textPart->isHtmlUsed() ) {
    toTry = textPart->cleanHtml();
  }
  foreach( const QByteArray &name, charsets ) {
    // We use KCharsets::codecForName() instead of QTextCodec::codecForName() here, because
    // the former knows us-ascii is latin1.
    QTextCodec *codec = KGlobal::charsets()->codecForName( QString::fromLatin1( name ) );
    if( !codec ) {
      kWarning() << "Could not get text codec for charset" << name;
      continue;
    }
    if( codec->canEncode( toTry ) ) {
      // Special check for us-ascii (needed because us-ascii is not exactly latin1).
      if( name == "us-ascii" && !isUsAscii( toTry ) ) {
        continue;
      }
      kDebug() << "Chosen charset" << name;
      chosenCharset = name;
      return true;
    }
  }
  kDebug() << "No appropriate charset found.";
  return false;
}

void MainTextJobPrivate::encodeTexts()
{
  Q_Q( MainTextJob );
  QTextCodec *codec = KGlobal::charsets()->codecForName( QString::fromLatin1( chosenCharset ) );
  if( !codec ) {
    kError() << "Could not get text codec for charset" << chosenCharset;
    q->setError( Job::BugError );
    q->setErrorText( i18n( "Could not get text codec for charset \"%1\".", QString::fromLatin1( chosenCharset ) ) );
    return;
  }
  encodedPlainText = codec->fromUnicode( sourcePlainText );
  encodedHtml = codec->fromUnicode( textPart->cleanHtml() );
  kDebug() << "Done.";
}



MainTextJob::MainTextJob( TextPart *textPart, QObject *parent )
  : Job( *new MainTextJobPrivate( this ), parent )
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
  Q_ASSERT( d->composer );

  // Word wrapping.
  if( d->composer->behaviour().isActionEnabled( Behaviour::UseWrapping ) ) {
    d->sourcePlainText = d->textPart->wrappedPlainText();
  } else {
    d->sourcePlainText = d->textPart->cleanPlainText();
  }

  // Charset.
  if( d->chooseCharsetAndEncode() ) {
    // Encoding was successful.  The user and we are happy with the charset, even if it may
    // lose characters.
    if( d->encodedHtml.isEmpty() ) {
      kDebug() << "Making text/plain";
      // Content is text/plain.
      ContentJob *cjob = new ContentJob( this );
      cjob->contentType()->setMimeType( "text/plain" );
      cjob->contentType()->setCharset( d->chosenCharset );
      cjob->setData( d->encodedPlainText );

      // TODO temporary until I figure out what the CTE policy is.
      cjob->contentTransferEncoding()->setEncoding( Headers::CEquPr );
    } else {
      // TODO Handle multipart/alternative and multipart/related.
      Q_ASSERT( false );
    }
    Job::doStart();
  } else {
    // chooseCharsetAndEncode has set an error.
    Q_ASSERT( error() );
    emitResult();
  }
}

void MainTextJob::process()
{
  Q_D( MainTextJob );
  // The content has been created by our subjob.
  Q_ASSERT( d->subjobContents.count() == 1 );
  d->resultContent = d->subjobContents.first();
  emitResult();
}

#include "maintextjob.moc"
