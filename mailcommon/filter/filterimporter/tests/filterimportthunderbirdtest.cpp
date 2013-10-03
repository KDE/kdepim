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
#include "mailfilter.h"
#include <qtest_kde.h>


QTEST_KDEMAIN( FilterImportThunderbirdtest, NoGUI )

void FilterImportThunderbirdtest::testImportFilters()
{
#if 0
    QString filter = QLatin1String("version=\"9\"\n"
                                   "logging=\"no\"\n"
                                   "name=\"Match All Messages\"\n"
                                   "enabled=\"yes\"\n"
                                   "type=\"17\"\n"
                                   "action=\"Mark read\"\n"
                                   "condition=\"ALL\"\n");
    MailCommon::FilterImporterThunderbird importer(filter);
    QList<MailCommon::MailFilter*> lst = importer.importFilter();
    QCOMPARE(lst.count(), 1);
    qDeleteAll(lst);
#endif
}


#include "filterimportthunderbirdtest.moc"
