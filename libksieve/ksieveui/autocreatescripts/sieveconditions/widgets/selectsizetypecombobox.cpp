/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "selectsizetypecombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLocalizedString>

using namespace KSieveUi;

SelectSizeTypeComboBox::SelectSizeTypeComboBox(QWidget *parent)
    : KComboBox(parent)
{
    initialize();
    connect(this, static_cast<void (SelectSizeTypeComboBox::*)(int)>(&SelectSizeTypeComboBox::activated), this, &SelectSizeTypeComboBox::valueChanged);
}

SelectSizeTypeComboBox::~SelectSizeTypeComboBox()
{
}

void SelectSizeTypeComboBox::initialize()
{
    addItem(i18n("Bytes"), QString());
    addItem(i18n("KB"), QLatin1String("K"));
    addItem(i18n("MB"), QLatin1String("M"));
    addItem(i18n("GB"), QLatin1String("G"));
}

QString SelectSizeTypeComboBox::code() const
{
    return itemData(currentIndex()).toString();
}

void SelectSizeTypeComboBox::setCode(const QString &code, const QString &name, QString &error)
{
    const int index = findData(code);
    if (index != -1) {
        setCurrentIndex(index);
    } else {
        AutoCreateScriptUtil::comboboxItemNotFound(code, name, error);
        setCurrentIndex(0);
    }
}

