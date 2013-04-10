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

#include "selectmatchtypecombobox.h"

#include <KLocale>

using namespace KSieveUi;

SelectMatchTypeComboBox::SelectMatchTypeComboBox(QWidget *parent)
    : KComboBox(parent)
{
    initialize();
}

SelectMatchTypeComboBox::~SelectMatchTypeComboBox()
{
}

void SelectMatchTypeComboBox::initialize()
{
    addItem(i18n("is"), QLatin1String(":is"));
    addItem(i18n("contains"), QLatin1String(":contains"));
    addItem(i18n("matches"), QLatin1String(":matches"));
}

QString SelectMatchTypeComboBox::code() const
{
    return itemData(currentIndex()).toString();
}

#include "selectmatchtypecombobox.moc"
