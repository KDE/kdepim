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
#include <qtest_kde.h>

#include <kmime/kmime_content.h>
using namespace KMime;

#include <messagecomposer/composer.h>
#include <messagecomposer/maintextjob.h>
#include <messagecomposer/textpart.h>
using namespace MessageComposer;

QTEST_KDEMAIN( MainTextJobTest, NoGUI )

void MainTextJobTest::testPlainText()
{
  Composer *composer = new Composer;
  composer->behaviour().disableAction( Behaviour::UseGui );
  TextPart *textPart = new TextPart;
  QString data = QString::fromLatin1( "they said their nevers they slept their dream" );
  QList<QByteArray> charsets;
  charsets << "us-ascii" << "utf-8";
  textPart->setWrappedPlainText( data );
  textPart->setCharsets( charsets );
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
    composer->behaviour().disableAction( Behaviour::UseGui );
    composer->behaviour().disableAction( Behaviour::UseWrapping );
    composer->behaviour().enableAction( Behaviour::UseFallbackCharset );
    TextPart *textPart = new TextPart;
    QString data = QString::fromLatin1( "they said their nevers they slept their dream" );
    textPart->setWrappedPlainText( data );
    MainTextJob *mjob = new MainTextJob( textPart, composer );
    QVERIFY( !mjob->exec() ); // error: not UseWrapping but given only wrapped text
    QCOMPARE( mjob->error(), int( Job::BugError ) );
  }
  {
    Composer *composer = new Composer;
    composer->behaviour().disableAction( Behaviour::UseGui );
    composer->behaviour().enableAction( Behaviour::UseWrapping );
    composer->behaviour().enableAction( Behaviour::UseFallbackCharset );
    TextPart *textPart = new TextPart;
    QString data = QString::fromLatin1( "they said their nevers they slept their dream" );
    textPart->setCleanPlainText( data );
    MainTextJob *mjob = new MainTextJob( textPart, composer );
    QVERIFY( !mjob->exec() ); // error: UseWrapping but given only clean text
    QCOMPARE( mjob->error(), int( Job::BugError ) );
  }
}

void MainTextJobTest::testCustomCharset()
{
  Composer *composer = new Composer;
  composer->behaviour().disableAction( Behaviour::UseGui );
  TextPart *textPart = new TextPart;
  QString data = QString::fromUtf8( "şi el o să se-nchidă cu o frunză de pelin" );
  QByteArray charset( "iso-8859-2" );
  textPart->setWrappedPlainText( data );
  textPart->setCharsets( QList<QByteArray>() << charset );
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
  composer->behaviour().disableAction( Behaviour::UseGui );
  TextPart *textPart = new TextPart;
  QString data = QString::fromLatin1( "do you still play the accordion?" );
  textPart->setWrappedPlainText( data );
  MainTextJob *mjob = new MainTextJob( textPart, composer );
  QVERIFY( !mjob->exec() ); // Error.
  QCOMPARE( mjob->error(), int( Job::BugError ) );
  kDebug() << mjob->errorString();
}

void MainTextJobTest::testBadCharset()
{
  Composer *composer = new Composer;
  composer->behaviour().disableAction( Behaviour::UseGui );
  TextPart *textPart = new TextPart;
  QString data = QString::fromUtf8( "el a plâns peste ţară cu lacrima limbii noastre" );
  QByteArray charset( "us-ascii" ); // Cannot handle Romanian chars.
  textPart->setWrappedPlainText( data );
  textPart->setCharsets( QList<QByteArray>() << charset );
  MainTextJob *mjob = new MainTextJob( textPart, composer );
  QVERIFY( !mjob->exec() ); // Error.
  QCOMPARE( mjob->error(), int( Job::UserError ) );
  kDebug() << mjob->errorString();
}

void MainTextJobTest::testFallbackCharset()
{
  Composer *composer = new Composer;
  composer->behaviour().disableAction( Behaviour::UseGui );
  composer->behaviour().enableAction( Behaviour::UseFallbackCharset );
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
  QCOMPARE( result->contentType()->charset(), QByteArray( "utf-8" ) ); // Fallback is UTF-8.
  QCOMPARE( QString::fromLatin1( result->body() ), data );
}

void MainTextJobTest::testOverrideCTE()
{
  Composer *composer = new Composer;
  QVERIFY( !composer->behaviour().isActionEnabled( Behaviour::EightBitTransport ) );
  composer->behaviour().enableAction( Behaviour::UseFallbackCharset );
  TextPart *textPart = new TextPart;

  // 8bit if asked for and allowed.
  {
    composer->behaviour().enableAction( Behaviour::EightBitTransport );
    QString data = QString::fromUtf8( "[ăîşţâ]" );
    textPart->setWrappedPlainText( data );
    // Force it to use an 8bit encoding:
    QByteArray charset( "iso-8859-2" );
    textPart->setCharsets( QList<QByteArray>() << charset );
    MainTextJob *mjob = new MainTextJob( textPart, composer );
    QVERIFY( mjob->exec() );
    Content *result = mjob->content();
    result->assemble();
    kDebug() << result->encodedContent();
    QVERIFY( result->contentTransferEncoding( false ) );
    QCOMPARE( result->contentTransferEncoding()->encoding(), Headers::CE8Bit );
    QTextCodec *codec = QTextCodec::codecForName( charset );
    QVERIFY( codec );
    QCOMPARE( codec->toUnicode( result->body() ), data );
  }
}

#include "maintextjobtest.moc"
