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

#include "potentialduplicatecontactswidget.h"
#include <KABC/Addressee>

PotentialDuplicateContactsWidget::PotentialDuplicateContactsWidget(QWidget *parent)
    : QWidget(parent)
{
}

PotentialDuplicateContactsWidget::~PotentialDuplicateContactsWidget()
{

}

void PotentialDuplicateContactsWidget::setAddressList(const Akonadi::Item::List &list)
{
    mContactList = list;
    searchDuplicateContact();
}

void PotentialDuplicateContactsWidget::searchDuplicateContact()
{
    if (mContactList.count() > 1) {

    }
    //TODO
}

bool PotentialDuplicateContactsWidget::isDuplicate(const Akonadi::Item &itemA, const Akonadi::Item &itemB)
{
    KABC::Addressee addressA = itemA.payload<KABC::Addressee>();
    KABC::Addressee addressB = itemB.payload<KABC::Addressee>();
    if (addressA.name() == addressB.name()) {
        return true;
    }
    if (addressA.nickName() == addressB.nickName()) {
        return true;
    }

    return false;
}
