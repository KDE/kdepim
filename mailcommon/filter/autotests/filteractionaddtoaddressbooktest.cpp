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

#include "filteractionaddtoaddressbooktest.h"
#include "../filteractions/filteractionaddtoaddressbook.h"
#include <qtest_kde.h>
#include <QWidget>
#include <QLabel>
#include <klineedit.h>
#include <widgets/minimumcombobox.h>
#include <akonadi/collectioncombobox.h>

FilterActionAddToAddressBookTest::FilterActionAddToAddressBookTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionAddToAddressBookTest::~FilterActionAddToAddressBookTest()
{

}

void FilterActionAddToAddressBookTest::shouldHaveDefaultValue()
{
    MailCommon::FilterActionAddToAddressBook filter;
    QWidget *w = filter.createParamWidget(0);

    PimCommon::MinimumComboBox *headerCombo = w->findChild<PimCommon::MinimumComboBox *>(QLatin1String("HeaderComboBox"));
    QVERIFY(headerCombo);

    QLabel *label = w->findChild<QLabel *>(QLatin1String("label_with_category"));
    QVERIFY(label);

    KLineEdit *categoryEdit = w->findChild<KLineEdit *>( QLatin1String("CategoryEdit") );
    QVERIFY(categoryEdit);

    label = w->findChild<QLabel *>(QLatin1String("label_in_addressbook"));
    QVERIFY(label);

    Akonadi::CollectionComboBox *collectionComboBox = w->findChild<Akonadi::CollectionComboBox *>(QLatin1String("AddressBookComboBox") );
    QVERIFY(collectionComboBox);
}

QTEST_KDEMAIN(FilterActionAddToAddressBookTest, GUI)
