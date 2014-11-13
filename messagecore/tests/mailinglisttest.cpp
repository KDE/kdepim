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
#include <qtest_kde.h>
#include <KConfigGroup>
#include <KGlobal>
#include <QDebug>

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
    KUrl::List lst;
    lst << KUrl(QLatin1String("http://www.kde.org")) << KUrl(QLatin1String("http://www.koffice.org"));
    ml.setPostUrls( lst );
    ml.setSubscribeUrls( lst );
    ml.setUnsubscribeUrls( lst );
    ml.setHelpUrls( lst );
    /* Note: mArchivedAtUrl deliberately not saved here as it refers to a single
   * instance of a message rather than an element of a general mailing list.
   * http://reviewboard.kde.org/r/1768/#review2783
   */
    //normal that we don't save it.
    //ml.setArchivedAtUrls(lst);
    ml.setArchiveUrls(lst);
    ml.setOwnerUrls(lst);
    ml.setId(QLatin1String("ID"));
    ml.setHandler(MessageCore::MailingList::Browser);


    KConfigGroup grp(KGlobal::config(), "testsettings");
    ml.writeConfig(grp);

    MessageCore::MailingList restoreMl;
    restoreMl.readConfig(grp);
    QCOMPARE(ml, restoreMl);
}

QTEST_KDEMAIN(MailingListTest, NoGUI)
