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

#include "skeletonmessagejobtest.h"

#include <KDebug>
#include <qtest_kde.h>

#include <kmime/kmime_message.h>
using namespace KMime;

#include <messagecomposer/composer.h>
#include <messagecomposer/infopart.h>
#include <messagecomposer/skeletonmessagejob.h>
using namespace MessageComposer;

QTEST_KDEMAIN( SkeletonMessageJobTest, NoGUI )

void SkeletonMessageJobTest::testSubject_data()
{
  QTest::addColumn<QString>( "subject" );

  QTest::newRow( "simple subject" ) << "Antaa virrata sateen...";
  QTest::newRow( "non-ascii subject" ) << "Muzicologă în bej, vând whisky și tequila, preț fix.";
  // NOTE: This works fine, but shows ????s in the debug output.  Why?
}

void SkeletonMessageJobTest::testSubject()
{
  // An InfoPart should belong to a Composer, even if we don't use the composer itself.
  Composer *composer = new Composer;
  InfoPart *infoPart = composer->infoPart();
  Q_ASSERT( infoPart );

  QFETCH( QString, subject );
  //kDebug() << subject;
  infoPart->setSubject( subject );
  SkeletonMessageJob *sjob = new SkeletonMessageJob( infoPart, composer );
  QVERIFY( sjob->exec() );
  Message *message = sjob->message();
  QVERIFY( message->subject( false ) );
  kDebug() << message->subject()->asUnicodeString();
  QCOMPARE( subject, message->subject()->asUnicodeString() );
}

void SkeletonMessageJobTest::testAddresses_data()
{
  QTest::addColumn<QString>( "from" );
  QTest::addColumn<QStringList>( "to" );
  QTest::addColumn<QStringList>( "cc" );
  QTest::addColumn<QStringList>( "bcc" );

  {
    QString from( "one@example.com" );
    QStringList to( "two@example.com" );
    QStringList cc( "three@example.com" );
    QStringList bcc( "four@example.com" );

    QTest::newRow( "simple single address" ) << from << to << cc << bcc;
  }

  {
    QString from( "one@example.com" );
    QStringList to( "two@example.com" );
    to << "two.two@example.com";
    QStringList cc( "three@example.com" );
    cc << "three.three@example.com";
    QStringList bcc( "four@example.com" );
    bcc << "four.four@example.com";

    QTest::newRow( "simple multi address" ) << from << to << cc << bcc;
  }

  {
    QString from( "Me <one@example.com>" );
    QStringList to( "You <two@example.com>" );
    to << "two.two@example.com";
    QStringList cc( "And you <three@example.com>" );
    cc << "three.three@example.com";
    QStringList bcc( "And you too <four@example.com>" );
    bcc << "four.four@example.com";

    QTest::newRow( "named multi address" ) << from << to << cc << bcc;
  }

  {
    QString from( "Şîşkin <one@example.com>" );
    QStringList to( "Ivan Turbincă <two@example.com>" );
    to << "two.two@example.com";
    QStringList cc( "Luceafărul <three@example.com>" );
    cc << "three.three@example.com";
    QStringList bcc( "Zburătorul <four@example.com>" );
    bcc << "four.four@example.com";

    QTest::newRow( "non-ascii named multi address" ) << from << to << cc << bcc;
  }
}

void SkeletonMessageJobTest::testAddresses()
{
  // An InfoPart should belong to a Composer, even if we don't use the composer itself.
  Composer *composer = new Composer;
  InfoPart *infoPart = composer->infoPart();
  Q_ASSERT( infoPart );

  QFETCH( QString, from );
  QFETCH( QStringList, to );
  QFETCH( QStringList, cc );
  QFETCH( QStringList, bcc );
  infoPart->setFrom( from );
  infoPart->setTo( to );
  infoPart->setCc( cc );
  infoPart->setBcc( bcc );
  SkeletonMessageJob *sjob = new SkeletonMessageJob( infoPart, composer );
  QVERIFY( sjob->exec() );
  Message *message = sjob->message();

  {
    QVERIFY( message->from( false ) );
    kDebug() << "From:" << message->from()->asUnicodeString();
    QCOMPARE( from, message->from()->asUnicodeString() );
  }

  {
    QVERIFY( message->to( false ) );
    kDebug() << "To:" << message->to()->asUnicodeString();
    foreach( const QString &addr, message->to()->prettyAddresses() ) {
      kDebug() << addr;
      QVERIFY( to.contains( addr ) );
      to.removeOne( addr );
    }
    QVERIFY( to.isEmpty() );
  }

  {
    QVERIFY( message->cc( false ) );
    kDebug() << "Cc:" << message->cc()->asUnicodeString();
    foreach( const QString &addr, message->cc()->prettyAddresses() ) {
      kDebug() << addr;
      QVERIFY( cc.contains( addr ) );
      cc.removeOne( addr );
    }
    QVERIFY( cc.isEmpty() );
  }

  {
    QVERIFY( message->bcc( false ) );
    kDebug() << "Bcc:" << message->bcc()->asUnicodeString();
    foreach( const QString &addr, message->bcc()->prettyAddresses() ) {
      kDebug() << addr;
      QVERIFY( bcc.contains( addr ) );
      bcc.removeOne( addr );
    }
    QVERIFY( bcc.isEmpty() );
  }
}

#include "skeletonmessagejobtest.moc"
