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

#include "composertest.h"

#include <KDebug>
#include <qtest_kde.h>

#include <kmime/kmime_headers.h>
using namespace KMime;

#include <messagecomposer/composer.h>
#include <messagecomposer/infopart.h>
#include <messagecomposer/textpart.h>
using namespace MessageComposer;

QTEST_KDEMAIN( ComposerTest, NoGUI )

void ComposerTest::testCTEErrors()
{
  // First test that the composer succeeds at all.
  {
    Composer *composer = new Composer;
    composer->behaviour().enableAction( Behaviour::UseFallbackCharset );
    composer->infoPart()->setFrom( QString::fromLatin1( "me@me.me" ) );
    composer->infoPart()->setTo( QStringList( QString::fromLatin1( "you@you.you" ) ) );
    composer->textPart()->setWrappedPlainText( QString::fromLatin1( "sample content" ) );
    QVERIFY( composer->exec() );
    QCOMPARE( composer->messages().count(), 1 );
    kDebug() << composer->messages().first()->message()->encodedContent();
  }

  // unsupported CTE -> error.
  {
    Composer *composer = new Composer;
    composer->behaviour().enableAction( Behaviour::UseFallbackCharset );
    composer->infoPart()->setFrom( QString::fromLatin1( "me@me.me" ) );
    composer->infoPart()->setTo( QStringList( QString::fromLatin1( "you@you.you" ) ) );
    composer->textPart()->setWrappedPlainText( QString::fromLatin1( "sample content" ) );
    composer->textPart()->setOverrideTransferEncoding( Headers::CEbinary );
    QVERIFY( !composer->exec() );
    QCOMPARE( composer->error(), int( Job::BugError ) );
    kDebug() << composer->errorString();
  }

  // 8bit part when not EightBitTransport -> error.
  {
    Composer *composer = new Composer;
    composer->behaviour().enableAction( Behaviour::UseFallbackCharset );
    composer->infoPart()->setFrom( QString::fromLatin1( "me@me.me" ) );
    composer->infoPart()->setTo( QStringList( QString::fromLatin1( "you@you.you" ) ) );
    composer->textPart()->setWrappedPlainText( QString::fromLatin1( "sample content" ) );
    composer->textPart()->setOverrideTransferEncoding( Headers::CE8Bit );
    QVERIFY( !composer->exec() );
    QCOMPARE( composer->error(), int( Job::BugError ) );
    kDebug() << composer->errorString();
  }
}

#include "composertest.moc"
