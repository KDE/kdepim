/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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
#include "filterimportprocmailtest.h"
#include "../filterimporterprocmail.h"
#include "filter/mailfilter.h"
#include <AkonadiCore/qtest_akonadi.h>
#include "filtertestkernel.h"
#include <MailCommon/MailKernel>

void FilterImportProcmailtest::initTestCase()
{
    AkonadiTest::checkTestIsIsolated();

    FilterTestKernel *kernel = new FilterTestKernel(this);
    CommonKernel->registerKernelIf(kernel);   //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf(kernel);   //SettingsIf is used in FolderTreeWidget
}

void FilterImportProcmailtest::testImportFilters()
{
#if 0
    const QString filter = QLatin1String("version=\"9\"\n"
                                         "logging=\"no\"\n"
                                         "name=\"Match All Messages\"\n"
                                         "enabled=\"yes\"\n"
                                         "type=\"17\"\n"
                                         "action=\"Mark read\"\n"
                                         "condition=\"ALL\"\n");
    MailCommon::FilterImporterProcmail importer(filter, false);
    QList<MailCommon::MailFilter *> lst = importer.importFilter();
    QCOMPARE(lst.count(), 1);
    MailCommon::MailFilter *f = lst.at(0);
    QVERIFY(f->isEnabled());
    QCOMPARE(f->name(), QStringLiteral("Match All Messages"));
    QCOMPARE(f->pattern()->op(), MailCommon::SearchPattern::OpAll);

    qDeleteAll(lst);
#endif
}

QTEST_AKONADIMAIN(FilterImportProcmailtest)
