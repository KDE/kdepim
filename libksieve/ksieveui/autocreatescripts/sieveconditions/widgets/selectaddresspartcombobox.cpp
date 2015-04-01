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

#include "selectaddresspartcombobox.h"
#include "autocreatescripts/sieveeditorgraphicalmodewidget.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLocalizedString>

using namespace KSieveUi;

SelectAddressPartComboBox::SelectAddressPartComboBox(QWidget *parent)
    : QComboBox(parent)
{
    mHasSubaddressCapability = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QStringLiteral("subaddress"));
    initialize();
    connect(this, static_cast<void (SelectAddressPartComboBox::*)(int)>(&SelectAddressPartComboBox::activated), this, &SelectAddressPartComboBox::valueChanged);
}

SelectAddressPartComboBox::~SelectAddressPartComboBox()
{
}

void SelectAddressPartComboBox::initialize()
{
    addItem(i18n("all"), QStringLiteral(":all"));
    addItem(i18n("localpart"), QStringLiteral(":localpart"));
    addItem(i18n("domain"), QStringLiteral(":domain"));
    if (mHasSubaddressCapability) {
        addItem(i18n("user"), QStringLiteral(":user"));
        addItem(i18n("detail"), QStringLiteral(":detail"));
    }
}

QString SelectAddressPartComboBox::code() const
{
    return itemData(currentIndex()).toString();
}

QString SelectAddressPartComboBox::extraRequire() const
{
    if (mHasSubaddressCapability) {
        return QStringLiteral("subaddress");
    }
    return QString();
}

void SelectAddressPartComboBox::setCode(const QString &code, const QString &name, QString &error)
{
    const int index = findData(code);
    if (index != -1) {
        setCurrentIndex(index);
    } else {
        AutoCreateScriptUtil::comboboxItemNotFound(code, name, error);
        setCurrentIndex(0);
    }
}

