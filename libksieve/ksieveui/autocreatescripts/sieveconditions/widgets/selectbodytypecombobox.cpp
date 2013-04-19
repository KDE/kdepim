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

#include "selectbodytypecombobox.h"

#include <KLocale>

using namespace KSieveUi;

SelectBodyTypeComboBox::SelectBodyTypeComboBox(QWidget *parent)
    : KComboBox(parent)
{
    initialize();
}

SelectBodyTypeComboBox::~SelectBodyTypeComboBox()
{
}

void SelectBodyTypeComboBox::initialize()
{
    addItem(i18n("raw"), QLatin1String(":raw"));
    addItem(i18n("content"), QLatin1String(":content"));
    addItem(i18n("text"), QLatin1String(":text"));
}

QString SelectBodyTypeComboBox::code() const
{
    const QString value = itemData(currentIndex()).toString();
    return value;
}

#include "selectbodytypecombobox.moc"
