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

#include "mailinglisttest.h"
#include "messagecore/misc/mailinglist.h"
#include <qtest.h>
#include <KConfigGroup>
#include <QDebug>
#include <KSharedConfig>

//TODO add test for static MailingList detect(  const KMime::Message::Ptr &message ); and static QString name( ... );

MailingListTest::MailingListTest(QObject *parent)
    : QObject(parent)
{

}

MailingListTest::~MailingListTest()
{

}

void MailingListTest::shouldHaveDefaultValue()
{
    MessageCore::MailingList ml;
    QVERIFY(ml.postUrls().isEmpty());
    QVERIFY(ml.subscribeUrls().isEmpty());
    QVERIFY(ml.unsubscribeUrls().isEmpty());
    QVERIFY(ml.helpUrls().isEmpty());
    QVERIFY(ml.archiveUrls().isEmpty());
    QVERIFY(ml.ownerUrls().isEmpty());
    QVERIFY(ml.archivedAtUrls().isEmpty());
    QVERIFY(ml.id().isEmpty());
    QVERIFY(ml.features() == MessageCore::MailingList::None);
    QVERIFY(ml.handler() == MessageCore::MailingList::KMail);
}

void MailingListTest::shouldRestoreFromSettings()
{
    MessageCore::MailingList ml;
    QList<QUrl> lst;
    lst << QUrl(QLatin1String("http://www.kde.org")) << QUrl(QLatin1String("http://www.koffice.org"));
    ml.setPostUrls( lst );
    lst << QUrl(QLatin1String("mailto://www.kde2.org")) << QUrl(QLatin1String("http://www.koffice2.org"));
    ml.setSubscribeUrls( lst );
    lst << QUrl(QLatin1String("mailto://www.kde3.org")) << QUrl(QLatin1String("http://www.koffice3.org"));
    ml.setUnsubscribeUrls( lst );
    lst << QUrl(QLatin1String("mailto://www.kde4.org")) << QUrl(QLatin1String("http://www.koffice4.org"));
    ml.setHelpUrls( lst );
    /* Note: mArchivedAtUrl deliberately not saved here as it refers to a single
   * instance of a message rather than an element of a general mailing list.
   * http://reviewboard.kde.org/r/1768/#review2783
   */
    //normal that we don't save it.
    //ml.setArchivedAtUrls(lst);
    lst << QUrl(QLatin1String("mailto://www.kde5.org")) << QUrl(QLatin1String("http://www.koffice5.org"));
    ml.setArchiveUrls(lst);
    lst << QUrl(QLatin1String("mailto://www.kde6.org")) << QUrl(QLatin1String("http://www.koffice6.org"));
    ml.setOwnerUrls(lst);
    ml.setId(QLatin1String("ID"));
    ml.setHandler(MessageCore::MailingList::Browser);


    KConfigGroup grp(KSharedConfig::openConfig(), "testsettings");
    ml.writeConfig(grp);

    MessageCore::MailingList restoreMl;
    restoreMl.readConfig(grp);
    QCOMPARE(ml, restoreMl);
}

void MailingListTest::shouldCopyReminderInfo()
{
    MessageCore::MailingList ml;
    QList<QUrl> lst;
    lst << QUrl(QLatin1String("http://www.kde.org")) << QUrl(QLatin1String("http://www.koffice.org"));
    ml.setPostUrls( lst );
    lst << QUrl(QLatin1String("http://www.kde2.org")) << QUrl(QLatin1String("http://www.koffice2.org"));
    ml.setSubscribeUrls( lst );
    lst << QUrl(QLatin1String("http://www.kde3.org")) << QUrl(QLatin1String("http://www.koffice3.org"));
    ml.setUnsubscribeUrls( lst );
    lst << QUrl(QLatin1String("http://www.kde4.org")) << QUrl(QLatin1String("http://www.koffice4.org"));
    ml.setHelpUrls( lst );
    lst << QUrl(QLatin1String("http://www.kde5.org")) << QUrl(QLatin1String("http://www.koffice5.org"));
    ml.setArchivedAtUrls(lst);
    lst << QUrl(QLatin1String("http://www.kde5.org")) << QUrl(QLatin1String("http://www.koffice6.org"));
    ml.setArchiveUrls(lst);
    lst << QUrl(QLatin1String("http://www.kde6.org")) << QUrl(QLatin1String("http://www.koffice6.org"));
    ml.setOwnerUrls(lst);
    ml.setPostUrls( lst );
    ml.setSubscribeUrls( lst );
    ml.setUnsubscribeUrls( lst );
    ml.setHelpUrls( lst );
    ml.setArchivedAtUrls(lst);
    ml.setArchiveUrls(lst);
    ml.setOwnerUrls(lst);
    ml.setId(QLatin1String("ID"));
    ml.setHandler(MessageCore::MailingList::Browser);

    MessageCore::MailingList restoreMl(ml);
    QCOMPARE(ml, restoreMl);
}

QTEST_MAIN(MailingListTest)
