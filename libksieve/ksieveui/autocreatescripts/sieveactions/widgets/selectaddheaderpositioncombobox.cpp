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

#include "selectaddheaderpositioncombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KComboBox>
#include <KLocalizedString>

using namespace KSieveUi;
SelectAddHeaderPositionCombobox::SelectAddHeaderPositionCombobox(QWidget *parent)
    : KComboBox(parent)
{
    initialize();
}

SelectAddHeaderPositionCombobox::~SelectAddHeaderPositionCombobox()
{
}

void SelectAddHeaderPositionCombobox::initialize()
{
    addItem(i18n("Insert at the beginning"), QString());
    addItem(i18n("Append at the end"), QLatin1String(":last"));
}

QString SelectAddHeaderPositionCombobox::code() const
{
    return itemData(currentIndex()).toString();
}

void SelectAddHeaderPositionCombobox::setCode(const QString &code, const QString &name, QString &error)
{
    const int index = findData(code);
    if (index != -1) {
        setCurrentIndex(index);
    } else {
        AutoCreateScriptUtil::comboboxItemNotFound(code, name, error);
        setCurrentIndex(0);
    }
}

