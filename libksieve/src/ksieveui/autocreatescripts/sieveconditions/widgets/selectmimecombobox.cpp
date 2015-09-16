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

#include "selectmimecombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLocalizedString>

using namespace KSieveUi;

SelectMimeComboBox::SelectMimeComboBox(QWidget *parent)
    : QComboBox(parent)
{
    initialize();
    connect(this, static_cast<void (SelectMimeComboBox::*)(int)>(&SelectMimeComboBox::activated), this, &SelectMimeComboBox::valueChanged);
}

SelectMimeComboBox::~SelectMimeComboBox()
{
}

void SelectMimeComboBox::initialize()
{
    addItem(i18n("Type"), QStringLiteral(":type"));
    addItem(i18n("Subtype"), QStringLiteral(":subtype"));
    addItem(i18n("Anychild"), QStringLiteral(":anychild"));
    addItem(i18n("Parameters"), QStringLiteral(":param"));
}

QString SelectMimeComboBox::code() const
{
    return QStringLiteral(":mime \"%1\"").arg(itemData(currentIndex()).toString());
}

QString SelectMimeComboBox::require() const
{
    return QStringLiteral("mime");
}

void SelectMimeComboBox::setCode(const QString &code, const QString &name, QString &error)
{
    const int index = findData(code);
    if (index != -1) {
        setCurrentIndex(index);
    } else {
        AutoCreateScriptUtil::comboboxItemNotFound(code, name, error);
        setCurrentIndex(0);
    }
}

