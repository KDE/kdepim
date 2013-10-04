/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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
#include "filterimportthunderbirdtest.h"
#include "../filterimporterthunderbird_p.h"
#include "filtertestkernel.h"
#include "mailfilter.h"
#include <qtest_kde.h>
#include <mailcommon/kernel/mailkernel.h>


QTEST_KDEMAIN( FilterImportThunderbirdtest, NoGUI )

void FilterImportThunderbirdtest::initTestCase()
{
    FilterTestKernel *kernel = new FilterTestKernel( this );
    CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf( kernel ); //SettingsIf is used in FolderTreeWidget
}

void FilterImportThunderbirdtest::testImportFiltersAllCondition()
{
    QString filter = QLatin1String("version=\"9\"\n"
                                   "logging=\"no\"\n"
                                   "name=\"Match All Messages\"\n"
                                   "enabled=\"yes\"\n"
                                   "type=\"17\"\n"
                                   "action=\"Mark read\"\n"
                                   "condition=\"ALL\"\n");
    MailCommon::FilterImporterThunderbird importer(filter, false);
    QList<MailCommon::MailFilter*> lst = importer.importFilter();
    QCOMPARE(lst.count(), 1);
    MailCommon::MailFilter *f = lst.at(0);
    QVERIFY(f->isEnabled());
    QCOMPARE(f->name(), QLatin1String("Match All Messages"));
    QCOMPARE(f->pattern()->op(), MailCommon::SearchPattern::OpAll);

    qDeleteAll(lst);
}

void FilterImportThunderbirdtest::testImportFiltersEmpty()
{
    QString filter;
    MailCommon::FilterImporterThunderbird importer(filter, false);
    QList<MailCommon::MailFilter*> lst = importer.importFilter();
    QCOMPARE(lst.count(), 0);
}

void FilterImportThunderbirdtest::testImportFiltersStopExecution()
{
    QString filter = QLatin1String("version=\"9\"\n"
                                   "logging=\"no\"\n"
                                   "name=\"Match All Messages\"\n"
                                   "enabled=\"yes\"\n"
                                   "type=\"17\"\n"
                                   "action=\"Stop execution\"\n"
                                   "action=\"Mark read\"\n"
                                   "condition=\"ALL\"\n");
    MailCommon::FilterImporterThunderbird importer(filter, false);
    QList<MailCommon::MailFilter*> lst = importer.importFilter();
    MailCommon::MailFilter *f = lst.at(0);
    QVERIFY(f->isEnabled());
    QCOMPARE(f->stopProcessingHere(), true);
    qDeleteAll(lst);
}

void FilterImportThunderbirdtest::testImportFiltersDisabled()
{
    QString filter = QLatin1String("version=\"9\"\n"
                                   "logging=\"no\"\n"
                                   "name=\"Match All Messages\"\n"
                                   "enabled=\"no\"\n"
                                   "type=\"17\"\n"
                                   "action=\"Stop execution\"\n"
                                   "action=\"Mark read\"\n"
                                   "condition=\"ALL\"\n");
    MailCommon::FilterImporterThunderbird importer(filter, false);
    QList<MailCommon::MailFilter*> lst = importer.importFilter();
    MailCommon::MailFilter *f = lst.at(0);
    QCOMPARE(f->isEnabled(), false);
    QCOMPARE(f->stopProcessingHere(), true);
    qDeleteAll(lst);
}

void FilterImportThunderbirdtest::testImportTwoFilters()
{
    QString filter = QLatin1String("version=\"9\"\n"
                                   "logging=\"no\"\n"
                                   "name=\"Subject contains: kde\"\n"
                                   "enabled=\"yes\"\n"
                                   "type=\"17\"\n"
                                   "action=\"Copy to folder\"\n"
                                   "actionValue=\"mailbox://kde@pop.kde.org/Inbox\"\n"
                                   "condition=\"AND (subject,contains,konqi)\"\n"
                                   "name=\"filter1\"\n"
                                   "enabled=\"yes\"\n"
                                   "type=\"17\"\n"
                                   "action=\"Copy to folder\"\n"
                                   "actionValue=\"mailbox://kde@pop.kde.org/Inbox\"\n"
                                   "action=\"Mark read\"\n"
                                   "condition=\"AND (subject,contains,kmail) AND (subject,contains,konqueror) AND (subject,contains,kf5) AND (subject,contains,qtcreator)\"\n");
    MailCommon::FilterImporterThunderbird importer(filter, false);
    QList<MailCommon::MailFilter*> lst = importer.importFilter();
    QCOMPARE(lst.count(), 2);
    MailCommon::MailFilter *f = lst.at(0);
    QCOMPARE(f->pattern()->op(), MailCommon::SearchPattern::OpAnd);
    QCOMPARE(f->isEnabled(), true);
    f = lst.at(1);
    QCOMPARE(f->pattern()->op(), MailCommon::SearchPattern::OpAnd);
    QCOMPARE(f->isEnabled(), true);
    QCOMPARE(f->pattern()->count(), 4);
    qDeleteAll(lst);
}
#include "filterimportthunderbirdtest.moc"
