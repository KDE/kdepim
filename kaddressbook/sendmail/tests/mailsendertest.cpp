/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "mailsendertest.h"
#include "mailsender.h"

#include <KABC/Addressee>

#include <qtest_kde.h>


MailSenderTest::MailSenderTest()
{
}

void MailSenderTest::shouldNotSendSignalWhenWeDontSelectItem()
{
    Akonadi::Item::List lst;
    KABMailSender::MailSender mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    QCOMPARE(spy.count(), 0);
}

void MailSenderTest::shouldNotSendSignalWhenNoValidAddressItem()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KABC::Addressee address;
    address.setName(QLatin1String("foo1"));
    lst <<item<<item;
    KABMailSender::MailSender mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    QCOMPARE(spy.count(), 0);
}

void MailSenderTest::shouldNotSendSignalWhenNoEmails()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KABC::Addressee address;
    address.setName(QLatin1String("foo1"));
    item.setPayload<KABC::Addressee>( address );
    lst <<item<<item;
    KABMailSender::MailSender mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    QCOMPARE(spy.count(), 0);
}

void MailSenderTest::shouldSendSignalWhenOneEmail()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KABC::Addressee address;
    address.setName(QLatin1String("foo1"));
    address.insertEmail(QLatin1String("foo@kde.org"), true);
    item.setPayload<KABC::Addressee>( address );
    lst <<item;
    KABMailSender::MailSender mailsender(lst);
    QSignalSpy spy(&mailsender, SIGNAL(sendMails(QStringList)));
    mailsender.start();
    //QCOMPARE(spy.count(), 1);
    //const QStringList resultLst = spy.at(0).at(0).value<QStringList>();
    //QCOMPARE(resultLst.count(), 1);
}


QTEST_KDEMAIN(MailSenderTest, NoGUI)
