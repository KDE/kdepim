/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "mailsenderjobtest.h"
#include "mailsenderjob.h"

#include <KABC/Addressee>

#include <qtest_kde.h>


MailSenderJobTest::MailSenderJobTest()
{
}

void MailSenderJobTest::shouldNotSendSignalWhenWeDontSelectItem()
{
    Akonadi::Item::List lst;
    KABMailSender::MailSenderJob mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    QCOMPARE(spy.count(), 0);
}

void MailSenderJobTest::shouldNotSendSignalWhenNoValidAddressItem()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KABC::Addressee address;
    address.setName(QLatin1String("foo1"));
    lst <<item<<item;
    KABMailSender::MailSenderJob mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    QCOMPARE(spy.count(), 0);
}

void MailSenderJobTest::shouldNotSendSignalWhenNoEmails()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KABC::Addressee address;
    address.setName(QLatin1String("foo1"));
    item.setPayload<KABC::Addressee>( address );
    lst <<item<<item;
    KABMailSender::MailSenderJob mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    QCOMPARE(spy.count(), 0);
}

void MailSenderJobTest::shouldSendSignalWhenOneEmail()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KABC::Addressee address;
    address.setName(QLatin1String("foo1"));
    address.insertEmail(QLatin1String("foo@kde.org"), true);
    item.setPayload<KABC::Addressee>( address );
    lst <<item;
    KABMailSender::MailSenderJob mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    QCOMPARE(spy.count(), 1);
    const QStringList resultLst = spy.at(0).at(0).value<QStringList>();
    QCOMPARE(resultLst.count(), 1);
}

void MailSenderJobTest::shouldNotSendTwiceEmails()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KABC::Addressee address;
    address.setName(QLatin1String("foo1"));
    address.insertEmail(QLatin1String("foo@kde.org"), true);
    item.setPayload<KABC::Addressee>( address );
    lst <<item<<item;
    KABMailSender::MailSenderJob mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    const QStringList resultLst = spy.at(0).at(0).value<QStringList>();
    QCOMPARE(resultLst.count(), 1);
}

void MailSenderJobTest::shouldNotAddInvalidEmail()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KABC::Addressee address;
    address.setName(QLatin1String("foo1"));
    //Invalid email
    address.insertEmail(QLatin1String("foo2"), true);
    item.setPayload<KABC::Addressee>( address );
    lst <<item<<item;
    KABMailSender::MailSenderJob mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    QCOMPARE(spy.count(), 0);
}

void MailSenderJobTest::shouldEmitSignalIfThereIsAValidEmail()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KABC::Addressee address;
    address.setName(QLatin1String("foo1"));
    //Invalid email
    address.insertEmail(QLatin1String("foo2"), true);
    item.setPayload<KABC::Addressee>( address );
    lst <<item;

    Akonadi::Item item2;
    KABC::Addressee address2;
    address2.setName(QLatin1String("foo2"));
    address2.insertEmail(QLatin1String("foo2@kde.org"), true);
    item2.setPayload<KABC::Addressee>( address2 );
    lst <<item2;

    Akonadi::Item item3;
    KABC::Addressee address3;
    address3.setName(QLatin1String("foo3"));
    address3.insertEmail(QLatin1String("foo3@"), true);
    item3.setPayload<KABC::Addressee>( address3 );
    lst <<item3;

    Akonadi::Item item4;
    KABC::Addressee address4;
    address4.setName(QLatin1String("foo4"));
    address4.insertEmail(QLatin1String("foo4@kde.org"), true);
    item4.setPayload<KABC::Addressee>( address4 );
    lst <<item4;

    KABMailSender::MailSenderJob mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    QCOMPARE(spy.count(), 1);
    const QStringList resultLst = spy.at(0).at(0).value<QStringList>();
    QCOMPARE(resultLst.count(), 2);
}

//TODO Create unittest for ContactGroup too

QTEST_KDEMAIN(MailSenderJobTest, NoGUI)
