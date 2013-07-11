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

#include "selectimportancecombobox.h"

#include <KComboBox>
#include <KLocale>

using namespace KSieveUi;

SelectImportanceCombobox::SelectImportanceCombobox(QWidget *parent)
    : KComboBox(parent)
{
    initialize();
}

SelectImportanceCombobox::~SelectImportanceCombobox()
{
}

void SelectImportanceCombobox::initialize()
{
    addItem(i18n("high importance"), QLatin1String("1"));
    addItem(i18n("normal importance"), QLatin1String("2"));
    addItem(i18n("low importance"), QLatin1String("3"));
}

QString SelectImportanceCombobox::code() const
{
    return itemData(currentIndex()).toString();
}

void SelectImportanceCombobox::setCode(const QString &code)
{
    const int index = findData(code);
    if (index != -1) {
        setCurrentIndex(index);
    } else {
        //TODO other value ?
        setCurrentIndex(0);
    }
}

#include "selectimportancecombobox.moc"
