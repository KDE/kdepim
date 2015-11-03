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

#include <QDebug>
#include <qtest.h>

#include <kmime/kmime_message.h>

#include <MessageComposer/Composer>
#include <MessageComposer/InfoPart>
#include <MessageComposer/GlobalPart>
#include <MessageComposer/SkeletonMessageJob>
using namespace MessageComposer;

QTEST_MAIN(SkeletonMessageJobTest)

void SkeletonMessageJobTest::testSubject_data()
{
    QTest::addColumn<QString>("subject");

    QTest::newRow("simple subject") << QStringLiteral("Antaa virrata sateen...");
    QTest::newRow("non-ascii subject") << QStringLiteral("Muzicologă în bej, vând whisky și tequila, preț fix.");
    // NOTE: This works fine, but shows ??s in the debug output.  Why?
}

void SkeletonMessageJobTest::testSubject()
{
    // An InfoPart should belong to a Composer, even if we don't use the composer itself.
    Composer *composer = new Composer;
    InfoPart *infoPart = composer->infoPart();
    GlobalPart *globalPart = composer->globalPart();
    Q_ASSERT(infoPart);

    QFETCH(QString, subject);
    //qDebug() << subject;
    infoPart->setSubject(subject);
    SkeletonMessageJob *sjob = new SkeletonMessageJob(infoPart, globalPart, composer);
    QVERIFY(sjob->exec());
    KMime::Message *message = sjob->message();
    QVERIFY(message->subject(false));
    qDebug() << message->subject()->asUnicodeString();
    QCOMPARE(subject, message->subject()->asUnicodeString());
}

void SkeletonMessageJobTest::testAddresses_data()
{
    QTest::addColumn<QString>("from");
    QTest::addColumn<QString>("replyto");
    QTest::addColumn<QStringList>("to");
    QTest::addColumn<QStringList>("cc");
    QTest::addColumn<QStringList>("bcc");

    {
        QString from = QStringLiteral("one@example.com");
        QStringList to;
        to << QStringLiteral("two@example.com");
        QStringList cc;
        cc << QStringLiteral("three@example.com");
        QStringList bcc;
        bcc << QStringLiteral("four@example.com");
        QString replyto = QStringLiteral("five@example.com");

        QTest::newRow("simple single address") << from << replyto << to << cc << bcc;
    }

    {
        QString from = QStringLiteral("one@example.com");
        QStringList to;
        to << QStringLiteral("two@example.com");
        to << QStringLiteral("two.two@example.com");
        QStringList cc;
        cc << QStringLiteral("three@example.com");
        cc << QStringLiteral("three.three@example.com");
        QStringList bcc;
        bcc << QStringLiteral("four@example.com");
        bcc << QStringLiteral("four.four@example.com");
        QString replyto = QStringLiteral("five@example.com");

        QTest::newRow("simple multi address") << from << replyto << to << cc << bcc;
    }

    {
        QString from = QStringLiteral("Me <one@example.com>");
        QStringList to;
        to << QStringLiteral("You <two@example.com>");
        to << QStringLiteral("two.two@example.com");
        QStringList cc;
        cc << QStringLiteral("And you <three@example.com>");
        cc << QStringLiteral("three.three@example.com");
        QStringList bcc;
        bcc << QStringLiteral("And you too <four@example.com>");
        bcc << QStringLiteral("four.four@example.com");
        QString replyto = QStringLiteral("You over there <five@example.com>");

        QTest::newRow("named multi address") << from << replyto << to << cc << bcc;
    }

    {
        QString from = QStringLiteral("Şîşkin <one@example.com>");
        QStringList to;
        to << QStringLiteral("Ivan Turbincă <two@example.com>");
        to << QStringLiteral("two.two@example.com");
        QStringList cc;
        cc << QStringLiteral("Luceafărul <three@example.com>");
        cc << QStringLiteral("three.three@example.com");
        QStringList bcc;
        bcc << QStringLiteral("Zburătorul <four@example.com>");
        bcc << QStringLiteral("four.four@example.com");
        QString replyto = QStringLiteral("Şîşzbură <five@example.com>");

        QTest::newRow("non-ascii named multi address") << from << replyto << to << cc << bcc;
    }
}

void SkeletonMessageJobTest::testAddresses()
{
    // An InfoPart should belong to a Composer, even if we don't use the composer itself.
    Composer *composer = new Composer;
    InfoPart *infoPart = composer->infoPart();
    GlobalPart *globalPart = composer->globalPart();
    Q_ASSERT(infoPart);

    QFETCH(QString, from);
    QFETCH(QString, replyto);
    QFETCH(QStringList, to);
    QFETCH(QStringList, cc);
    QFETCH(QStringList, bcc);
    infoPart->setFrom(from);
    infoPart->setReplyTo(replyto);
    infoPart->setTo(to);
    infoPart->setCc(cc);
    infoPart->setBcc(bcc);
    SkeletonMessageJob *sjob = new SkeletonMessageJob(infoPart, globalPart, composer);
    QVERIFY(sjob->exec());
    KMime::Message *message = sjob->message();

    {
        QVERIFY(message->from(false));
        qDebug() << "From:" << message->from()->asUnicodeString();
        QCOMPARE(from, message->from()->asUnicodeString());
    }

    {
        QVERIFY(message->replyTo(false));
        qDebug() << "Reply-To:" << message->replyTo()->asUnicodeString();
        QCOMPARE(replyto, message->replyTo()->asUnicodeString());
    }

    {
        QVERIFY(message->to(false));
        qDebug() << "To:" << message->to()->asUnicodeString();
        foreach (const auto &addr, message->to()->mailboxes()) {
            qDebug() << addr.prettyAddress();
            QVERIFY(to.contains(addr.prettyAddress()));
            to.removeOne(addr.prettyAddress());
        }
        QVERIFY(to.isEmpty());
    }

    {
        QVERIFY(message->cc(false));
        qDebug() << "Cc:" << message->cc()->asUnicodeString();
        foreach (const auto &addr, message->cc()->mailboxes()) {
            qDebug() << addr.prettyAddress();
            QVERIFY(cc.contains(addr.prettyAddress()));
            cc.removeOne(addr.prettyAddress());
        }
        QVERIFY(cc.isEmpty());
    }

    {
        QVERIFY(message->bcc(false));
        qDebug() << "Bcc:" << message->bcc()->asUnicodeString();
        foreach (const auto &addr, message->bcc()->mailboxes()) {
            qDebug() << addr.prettyAddress();
            QVERIFY(bcc.contains(addr.prettyAddress()));
            bcc.removeOne(addr.prettyAddress());
        }
        QVERIFY(bcc.isEmpty());
    }
}

void SkeletonMessageJobTest::testMessageID()
{
    Composer *composer = new Composer();
    InfoPart *infoPart = composer->infoPart();
    GlobalPart *globalPart = composer->globalPart();
    Q_ASSERT(infoPart);

    SkeletonMessageJob *sjob = new SkeletonMessageJob(infoPart, globalPart, composer);
    QVERIFY(sjob->exec());
    KMime::Message *message = sjob->message();
    QVERIFY(message->messageID(false));
    QVERIFY(!message->messageID(false)->isEmpty());
    delete sjob;
    delete composer;
}

