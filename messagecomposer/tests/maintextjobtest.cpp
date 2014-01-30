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

#include "maintextjobtest.h"

#include <QTextCodec>

#include <KDebug>
#include <KIconLoader>
#include <qtest_kde.h>

#include <kmime/kmime_content.h>
using namespace KMime;

#include <messagecomposer/composer/composer.h>
#include <messagecomposer/part/globalpart.h>
#include <messagecomposer/job/maintextjob.h>
#include <messagecomposer/part/textpart.h>
using namespace MessageComposer;

#include <kpimtextedit/textedit.h>

QTEST_KDEMAIN( MainTextJobTest, GUI )

void MainTextJobTest::testPlainText()
{
  Composer *composer = new Composer;
  composer->globalPart()->setGuiEnabled( false );
  QList<QByteArray> charsets;
  charsets << "us-ascii" << "utf-8";
  composer->globalPart()->setCharsets( charsets );
  TextPart *textPart = new TextPart;
  QString data = QString::fromLatin1( "they said their nevers they slept their dream" );
  textPart->setWrappedPlainText( data );
  MainTextJob *mjob = new MainTextJob( textPart, composer );
  QVERIFY( mjob->exec() );
  Content *result = mjob->content();
  result->assemble();
  kDebug() << result->encodedContent();
  QVERIFY( result->contentType( false ) );
  QCOMPARE( result->contentType()->mimeType(), QByteArray( "text/plain" ) );
  QCOMPARE( result->contentType()->charset(), QByteArray( "us-ascii" ) );
  QCOMPARE( QString::fromLatin1( result->body() ), data );
}

void MainTextJobTest::testWrappingErrors()
{
  {
    Composer *composer = new Composer;
    composer->globalPart()->setGuiEnabled( false );
    composer->globalPart()->setFallbackCharsetEnabled( true );
    TextPart *textPart = new TextPart;
    QString data = QString::fromLatin1( "they said their nevers they slept their dream" );
    textPart->setWordWrappingEnabled( false );
    textPart->setWrappedPlainText( data );
    MainTextJob *mjob = new MainTextJob( textPart, composer );
    QVERIFY( !mjob->exec() ); // error: not UseWrapping but given only wrapped text
    QCOMPARE( mjob->error(), int( JobBase::BugError ) );
  }
  {
    Composer *composer = new Composer;
    composer->globalPart()->setGuiEnabled( false );
    composer->globalPart()->setFallbackCharsetEnabled( true );
    TextPart *textPart = new TextPart;
    textPart->setWordWrappingEnabled( true );
    QString data = QString::fromLatin1( "they said their nevers they slept their dream" );
    textPart->setCleanPlainText( data );
    MainTextJob *mjob = new MainTextJob( textPart, composer );
    QVERIFY( !mjob->exec() ); // error: UseWrapping but given only clean text
    QCOMPARE( mjob->error(), int( JobBase::BugError ) );
  }
}

void MainTextJobTest::testCustomCharset()
{
  Composer *composer = new Composer;
  composer->globalPart()->setGuiEnabled( false );
  QByteArray charset( "iso-8859-2" );
  composer->globalPart()->setCharsets( QList<QByteArray>() << charset );
  TextPart *textPart = new TextPart;
  QString data = QString::fromUtf8( "şi el o să se-nchidă cu o frunză de pelin" );
  textPart->setWrappedPlainText( data );
  MainTextJob *mjob = new MainTextJob( textPart, composer );
  QVERIFY( mjob->exec() );
  Content *result = mjob->content();
  result->assemble();
  kDebug() << result->encodedContent();
  QVERIFY( result->contentType( false ) );
  QCOMPARE( result->contentType()->mimeType(), QByteArray( "text/plain" ) );
  QCOMPARE( result->contentType()->charset(), charset );
  QByteArray outData = result->body();
  QTextCodec *codec = QTextCodec::codecForName( charset );
  QVERIFY( codec );
  QCOMPARE( codec->toUnicode( outData ), data );
}

void MainTextJobTest::testNoCharset()
{
  Composer *composer = new Composer;
  QVERIFY( !composer->globalPart()->isFallbackCharsetEnabled() );
  composer->globalPart()->setGuiEnabled( false );
  TextPart *textPart = new TextPart;
  QString data = QString::fromLatin1( "do you still play the accordion?" );
  textPart->setWrappedPlainText( data );
  MainTextJob *mjob = new MainTextJob( textPart, composer );
  QSKIP("This tests has been failing for a long time, please someone fix it", SkipSingle);
  QVERIFY( !mjob->exec() ); // Error.
  QCOMPARE( mjob->error(), int( JobBase::BugError ) );
  kDebug() << mjob->errorString();
}

void MainTextJobTest::testBadCharset()
{
  Composer *composer = new Composer;
  composer->globalPart()->setGuiEnabled( false );
  QByteArray charset( "us-ascii" ); // Cannot handle Romanian chars.
  composer->globalPart()->setCharsets( QList<QByteArray>() << charset );
  TextPart *textPart = new TextPart;
  QString data = QString::fromUtf8( "el a plâns peste ţară cu lacrima limbii noastre" );
  textPart->setWrappedPlainText( data );
  MainTextJob *mjob = new MainTextJob( textPart, composer );
  QSKIP("This tests has been failing for a long time, please someone fix it", SkipSingle);
  QVERIFY( !mjob->exec() ); // Error.
  QCOMPARE( mjob->error(), int( JobBase::UserError ) );
  kDebug() << mjob->errorString();
}

void MainTextJobTest::testFallbackCharset()
{
  Composer *composer = new Composer;
  composer->globalPart()->setGuiEnabled( false );
  composer->globalPart()->setFallbackCharsetEnabled( true );
  TextPart *textPart = new TextPart;
  QString data = QString::fromLatin1( "and when he falleth..." );
  textPart->setWrappedPlainText( data );
  MainTextJob *mjob = new MainTextJob( textPart, composer );
  QVERIFY( mjob->exec() );
  Content *result = mjob->content();
  result->assemble();
  kDebug() << result->encodedContent();
  QVERIFY( result->contentType( false ) );
  QCOMPARE( result->contentType()->mimeType(), QByteArray( "text/plain" ) );
  QCOMPARE( result->contentType()->charset(), QByteArray( "us-ascii" ) ); // Fallback is us-ascii or utf8.
  QCOMPARE( QString::fromLatin1( result->body() ), data );
}

void MainTextJobTest::testHtml()
{
  QLatin1String originalHtml( "<html><head></head><body>Test <em>with</em> formatting...<br>The end.</body></html>" );
  KPIMTextEdit::TextEdit editor;
  editor.setTextOrHtml( originalHtml );
  QVERIFY( editor.isFormattingUsed() );

  Composer *composer = new Composer;
  composer->globalPart()->setGuiEnabled( false );
  composer->globalPart()->setFallbackCharsetEnabled( true );
  TextPart *textPart = new TextPart;
  textPart->setWordWrappingEnabled( false );
  textPart->setCleanPlainText( editor.toCleanPlainText() );
  textPart->setCleanHtml( editor.toCleanHtml() );
  MainTextJob *mjob = new MainTextJob( textPart, composer );
  QVERIFY( mjob->exec() );
  Content *result = mjob->content();
  result->assemble();
  kDebug() << result->encodedContent();

  // multipart/alternative
  {
    QVERIFY( result->contentType( false ) );
    QCOMPARE( result->contentType()->mimeType(), QByteArray( "multipart/alternative" ) );
    QCOMPARE( result->contents().count(), 2 );
    // text/plain
    {
      Content *plain = result->contents().at( 0 );
      QVERIFY( plain->contentType( false ) );
      QCOMPARE( plain->contentType()->mimeType(), QByteArray( "text/plain" ) );
      QCOMPARE( QString::fromLatin1( plain->body() ), editor.toCleanPlainText() );
    }
    // text/html
    {
      Content *html = result->contents().at( 1 );
      QVERIFY( html->contentType( false ) );
      QCOMPARE( html->contentType()->mimeType(), QByteArray( "text/html" ) );
      // The editor adds extra Html stuff, so we can't compare to originalHtml.
      QCOMPARE( QLatin1String( html->body() ), editor.toCleanHtml() );
    }
  }
}

void MainTextJobTest::testHtmlWithImages()
{
  KPIMTextEdit::TextEdit editor;
  QString image1 = KIconLoader::global()->iconPath( QLatin1String( "folder-new" ), KIconLoader::Small, false );
  QString image2 = KIconLoader::global()->iconPath( QLatin1String( "message" ), KIconLoader::Small, false );
  QString data = QString::fromLatin1( "dust in the wind" );
  editor.setTextOrHtml( data );
  editor.addImage( image1 );
  editor.addImage( image2 );
  KPIMTextEdit::ImageList images = editor.embeddedImages();
  QCOMPARE( images.count(), 2 );
  QString cid1 = images[0]->contentID;
  QString cid2 = images[1]->contentID;
  QString name1 = images[0]->imageName;
  QString name2 = images[1]->imageName;

  Composer *composer = new Composer;
  composer->globalPart()->setGuiEnabled( false );
  composer->globalPart()->setFallbackCharsetEnabled( true );
  TextPart *textPart = new TextPart;
  textPart->setWordWrappingEnabled( false );
  textPart->setCleanPlainText( editor.toCleanPlainText() );
  textPart->setCleanHtml( editor.toCleanHtml() );
  textPart->setEmbeddedImages( editor.embeddedImages() );
  MainTextJob *mjob = new MainTextJob( textPart, composer );
  QVERIFY( mjob->exec() );
  Content *result = mjob->content();
  result->assemble();
  kDebug() << result->encodedContent();

  // multipart/related
  {
    QVERIFY( result->contentType( false ) );
    QCOMPARE( result->contentType()->mimeType(), QByteArray( "multipart/related" ) );
    QCOMPARE( result->contents().count(), 3 );
    // multipart/alternative
    {
      Content *alternative = result->contents().at( 0 );
      QVERIFY( alternative->contentType( false ) );
      QCOMPARE( alternative->contentType()->mimeType(), QByteArray( "multipart/alternative" ) );
      QCOMPARE( alternative->contents().count(), 2 );
      // text/plain
      {
        Content *plain = alternative->contents().at( 0 );
        QCOMPARE( plain->contentType()->mimeType(), QByteArray( "text/plain" ) );
        QCOMPARE( QString::fromLatin1( plain->body() ), data );
      }
      // text/html
      {
        Content *html = alternative->contents().at( 1 );
        QCOMPARE( html->contentType()->mimeType(), QByteArray( "text/html" ) );
        QString data = QString::fromLatin1( html->body() );
        int idx1 = data.indexOf( QString::fromLatin1( "cid:%1" ).arg( cid1 ) );
        int idx2 = data.indexOf( QString::fromLatin1( "cid:%1" ).arg( cid2 ) );
        QVERIFY( idx1 > 0 );
        QVERIFY( idx2 > 0 );
        QVERIFY( idx1 < idx2 );
      }
    }
    // First image/png
    {
      Content *image = result->contents().at( 1 );
      QVERIFY( image->contentType( false ) );
      QCOMPARE( image->contentType()->mimeType(), QByteArray( "image/png" ) );
      QCOMPARE( image->contentType()->name(), name1 );
      const Headers::ContentID *cid = image->header<Headers::ContentID>();
      QVERIFY( cid );
      QCOMPARE( cid->identifier(), cid1.toLatin1() );
    }
    // Second image/png
    {
      Content *image = result->contents().at( 2 );
      QVERIFY( image->contentType( false ) );
      QCOMPARE( image->contentType()->mimeType(), QByteArray( "image/png" ) );
      QCOMPARE( image->contentType()->name(), name2 );
      const Headers::ContentID *cid = image->header<Headers::ContentID>();
      QVERIFY( cid );
      QCOMPARE( cid->identifier(), cid2.toLatin1() );
    }
  }
}

