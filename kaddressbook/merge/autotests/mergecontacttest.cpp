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

#include "mergecontacttest.h"
#include <qtest.h>
#include "../job/mergecontacts.h"

using namespace KABMergeContacts;

MergeContactTest::MergeContactTest()
{
}

void MergeContactTest::shouldReturnDefaultAddressWhenNoListItem()
{
    MergeContacts contacts;
    KContacts::Addressee result = contacts.mergedContact();
    QCOMPARE(result.isEmpty(), true);
}

void MergeContactTest::shouldReturnDefaultAddressWhenOneItem()
{
    Akonadi::Item::List lst;
    Akonadi::Item item;
    KContacts::Addressee address;
    address.setName(QStringLiteral("foo1"));
    item.setPayload<KContacts::Addressee>(address);
    lst << item;

    MergeContacts contacts(lst);
    KContacts::Addressee result = contacts.mergedContact();
    QCOMPARE(result.isEmpty(), true);
}

void MergeContactTest::noNneedManualSelectionCheckWhenEmptyList()
{
    MergeContacts contacts;
    const bool result = contacts.needManualSelectInformations();
    QCOMPARE(result, false);
}

void MergeContactTest::noNneedManualSelectionCheckWhenOneItem()
{
    Akonadi::Item::List lst;
    KContacts::Addressee address;
    Akonadi::Item item;
    address.setName(QStringLiteral("foo1"));
    item.setPayload<KContacts::Addressee>(address);
    lst << item;
    MergeContacts contacts(lst);
    const bool result = contacts.needManualSelectInformations();
    QCOMPARE(result, false);
}

QTEST_MAIN(MergeContactTest)
