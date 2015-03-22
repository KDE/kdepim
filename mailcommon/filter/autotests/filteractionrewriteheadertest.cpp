/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "filteractionrewriteheadertest.h"
#include <qtest.h>
#include "../filteractions/filteractionrewriteheader.h"
#include <KLineEdit>
#include <QLabel>
#include <QWidget>
#include <widgets/minimumcombobox.h>
#include <widgets/regexplineedit.h>
FilterActionRewriteHeaderTest::FilterActionRewriteHeaderTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionRewriteHeaderTest::~FilterActionRewriteHeaderTest()
{

}

void FilterActionRewriteHeaderTest::shouldHaveDefaultValue()
{
    MailCommon::FilterActionRewriteHeader filter;
    QWidget *w = filter.createParamWidget(0);
    PimCommon::MinimumComboBox *combo = w->findChild<PimCommon::MinimumComboBox *>(QLatin1String("combo"));
    QVERIFY(combo);

    QLabel *label = w->findChild<QLabel *>(QLatin1String("label_replace"));
    QVERIFY(label);

    MailCommon::RegExpLineEdit *regExpLineEdit = w->findChild<MailCommon::RegExpLineEdit *>(QLatin1String("search") );
    QVERIFY(regExpLineEdit);
    QVERIFY(regExpLineEdit->text().isEmpty());

    label = w->findChild<QLabel *>(QLatin1String("label_with"));
    QVERIFY(label);

    KLineEdit *lineEdit = w->findChild<KLineEdit *>(QLatin1String("replace") );
    QVERIFY(lineEdit);
    QVERIFY(lineEdit->text().isEmpty());
}

QTEST_MAIN(FilterActionRewriteHeaderTest)
