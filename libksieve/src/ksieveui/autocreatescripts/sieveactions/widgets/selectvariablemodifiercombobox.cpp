/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "selectvariablemodifiercombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLocalizedString>

using namespace KSieveUi;

SelectVariableModifierComboBox::SelectVariableModifierComboBox(QWidget *parent)
    : KComboBox(parent)
{
    initialize();
    connect(this, static_cast<void (SelectVariableModifierComboBox::*)(int)>(&SelectVariableModifierComboBox::activated), this, &SelectVariableModifierComboBox::valueChanged);
}

SelectVariableModifierComboBox::~SelectVariableModifierComboBox()
{
}

void SelectVariableModifierComboBox::initialize()
{
    addItem(i18n("None"), QString());
    addItem(i18n("Lower"), QStringLiteral(":lower"));
    addItem(i18n("Upper"), QStringLiteral(":upper"));
    addItem(i18n("Lower first letter"), QStringLiteral(":lowerfirst"));
    addItem(i18n("Upper first letter"), QStringLiteral(":upperfirst"));
    addItem(i18n("Quote wildcard"), QStringLiteral(":quotewildcard"));
    addItem(i18n("Length"), QStringLiteral(":length"));
}

QString SelectVariableModifierComboBox::code() const
{
    return itemData(currentIndex()).toString();
}

void SelectVariableModifierComboBox::setCode(const QString &code, const QString &name, QString &error)
{
    const int index = findData(code);
    if (index != -1) {
        setCurrentIndex(index);
    } else {
        AutoCreateScriptUtil::comboboxItemNotFound(code, name, error);
        setCurrentIndex(0);
    }
}

