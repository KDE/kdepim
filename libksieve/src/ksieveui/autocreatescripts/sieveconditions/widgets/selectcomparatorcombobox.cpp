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

#include "selectcomparatorcombobox.h"
#include "autocreatescripts/sieveeditorgraphicalmodewidget.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

using namespace KSieveUi;
SelectComparatorComboBox::SelectComparatorComboBox(QWidget *parent)
    : QComboBox(parent)
{
    connect(this, static_cast<void (SelectComparatorComboBox::*)(int)>(&SelectComparatorComboBox::activated), this, &SelectComparatorComboBox::valueChanged);
    initialize();
}

SelectComparatorComboBox::~SelectComparatorComboBox()
{
}

void SelectComparatorComboBox::initialize()
{
    //Default in spec
    addItem(QStringLiteral("i;octet"), QStringLiteral("comparator-i;octet"));
    addItem(QStringLiteral("i;ascii-casemap"), QStringLiteral("comparator-i;ascii-casemap"));

    Q_FOREACH (const QString &capability, KSieveUi::SieveEditorGraphicalModeWidget::sieveCapabilities()) {
        if (capability.startsWith(QStringLiteral("comparator-"))) {
            QString str(capability);
            str.remove(QStringLiteral("comparator-"));
            addItem(str, capability);
        }
    }
}

QString SelectComparatorComboBox::require() const
{
    if (currentIndex() < 2) {
        return QString();
    }
    return itemData(currentIndex()).toString();
}

QString SelectComparatorComboBox::code() const
{
    return QStringLiteral(":comparator \"%1\"").arg(itemText(currentIndex()));
}

void SelectComparatorComboBox::setCode(const QString &code, const QString &name, QString &error)
{
    const QString completCode = QLatin1String("comparator-") + code;
    const int index = findData(completCode);
    if (index != -1) {
        setCurrentIndex(index);
    } else {
        AutoCreateScriptUtil::comboboxItemNotFound(code, name, error);
        setCurrentIndex(0);
    }
}

