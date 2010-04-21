/*
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>

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

#include "messagefactorytest.h"

#include <messagecore/stringutil.h>
#include "messagecomposer/composer.h"
#include "messagecomposer/messagefactory.h"
#include "messagecomposer/globalpart.h"
#include "messagecomposer/infopart.h"
#include "messagecomposer/textpart.h"
#include "qtest_messagecomposer.h"

#include <kpimidentities/identitymanager.h>
#include <qtest_kde.h>
#include <QDateTime>
using namespace Message;

QTEST_KDEMAIN( MessageFactoryTest, NoGUI )

void MessageFactoryTest::testCreateReply()
{
  KMime::Message::Ptr msg = createTestMessage();
  KPIMIdentities::IdentityManager* identMan = new KPIMIdentities::IdentityManager;

  MessageFactory factory( msg, 0 );
  factory.setIdentityManager( identMan );

  MessageFactory::MessageReply reply =  factory.createReply();
  QVERIFY( reply.replyAll = true );
  kDebug() << reply.msg->body();

  QDateTime date = msg->date()->dateTime().dateTime();
  QString datetime = KGlobal::locale()->formatDate( date.date(), KLocale::LongDate );
  datetime += QLatin1String( " " ) + KGlobal::locale()->formatTime( date.time(), true );
  QString replyStr = QString::fromLatin1( "On " + datetime.toLatin1() + " you wrote:\n> All happy families are alike; each unhappy family is unhappy in its own way.\n" );
  QVERIFY( reply.msg->subject()->asUnicodeString() == QLatin1String( "Re: Test Email Subject" ) );
  QVERIFY( reply.msg->body() == replyStr.toLatin1() );
  
}

void MessageFactoryTest::testCreateForward()
{
  KMime::Message::Ptr msg = createTestMessage();
  KPIMIdentities::IdentityManager* identMan = new KPIMIdentities::IdentityManager;

  MessageFactory factory( msg, 0 );
  factory.setIdentityManager( identMan );

  KMime::Message::Ptr fw =  factory.createForward();

  QDateTime date = msg->date()->dateTime().dateTime();
  QString datetime = KGlobal::locale()->formatDate( date.date(), KLocale::LongDate );
  datetime += QLatin1String( ", " ) + KGlobal::locale()->formatTime( date.time(), true );

  QString fwdMsg = QString::fromLatin1( "\n"
                                        "----------  Forwarded Message  ----------\n"
                      "\n"
                      "Subject: Test Email Subject\n"
                      "Date: %1\n"
                      "From: me@me.me\n"
                      "To: you@you.you\n"
                      "\n"
                      "All happy families are alike; each unhappy family is unhappy in its own way.\n"
                      "-----------------------------------------" );
  fwdMsg = fwdMsg.arg( datetime );
  
  QString fwdStr = QString::fromLatin1( "On " + datetime.toLatin1() + " you wrote:\n> All happy families are alike; each unhappy family is unhappy in its own way.\n" );
  QVERIFY( fw->subject()->asUnicodeString() == QLatin1String( "Fwd: Test Email Subject" ) );
  QVERIFY( fw->body() == fwdMsg.toLatin1() );

}


void MessageFactoryTest::testCreateRedirect()
{
  KMime::Message::Ptr msg = createTestMessage();
  KPIMIdentities::IdentityManager* identMan = new KPIMIdentities::IdentityManager;

  MessageFactory factory( msg, 0 );
  factory.setIdentityManager( identMan );

  QString redirectTo = QLatin1String("redir@redir.com");
  KMime::Message::Ptr rdir =  factory.createRedirect( redirectTo );

  QDateTime date = rdir->date()->dateTime().dateTime();
  QString datetime = KGlobal::locale()->formatDate( date.date(), KLocale::LongDate );
  datetime = rdir->date()->asUnicodeString();

  kDebug() << rdir->encodedContent();
  
  QString msgId = MessageCore::StringUtil::generateMessageId( msg->sender()->asUnicodeString(), QString() );
      
  QString baseline = QString::fromLatin1( "From: me@me.me\n"
                                          "Subject: Test Email Subject\n"
                                          "Date: %2\n"
                                          "MIME-Version: 1.0\n"
                                          "Content-Transfer-Encoding: 7Bit\n"
                                          "Content-Type: text/plain; charset=\"us-ascii\"\n"
                                          "Resent-Message-ID: %3\n"
                                          "Resent-Date: %4\n"
                                          "To: %1\n"
                                          "Resent-To:  <>\n"
                                          "X-KMail-Redirect-From: me@me.me (by way of  <>)\n"
                                          "X-KMail-Recipients: redir@redir.com\n"
                                          "\n"
                                          "All happy families are alike; each unhappy family is unhappy in its own way." );
  baseline = baseline.arg( redirectTo ).arg( datetime ).arg( msgId ).arg( datetime );

  kDebug() << baseline.toLatin1();

//   QString fwdStr = QString::fromLatin1( "On " + datetime.toLatin1() + " you wrote:\n> All happy families are alike; each unhappy family is unhappy in its own way.\n" );
  QVERIFY( rdir->subject()->asUnicodeString() == QLatin1String( "Test Email Subject" ) );
  QVERIFY( rdir->encodedContent() == baseline.toLatin1() );

}

KMime::Message::Ptr MessageFactoryTest::createTestMessage()
{
  Composer *composer = new Composer;
  composer->globalPart()->setFallbackCharsetEnabled( true );
  composer->infoPart()->setFrom( QString::fromLatin1( "me@me.me" ) );
  composer->infoPart()->setTo( QStringList( QString::fromLatin1( "you@you.you" ) ) );
  composer->textPart()->setWrappedPlainText( QString::fromLatin1( "All happy families are alike; each unhappy family is unhappy in its own way." ) );
  composer->infoPart()->setSubject( QLatin1String( "Test Email Subject" ) );
  composer->exec();
  
  KMime::Message::Ptr message = KMime::Message::Ptr( composer->resultMessages().first() );
  delete composer;

  return message;
}