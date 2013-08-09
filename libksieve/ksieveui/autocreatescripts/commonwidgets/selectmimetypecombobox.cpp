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

#include "selectmimetypecombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLocale>

using namespace KSieveUi;

SelectMimeTypeComboBox::SelectMimeTypeComboBox(QWidget *parent)
    : KComboBox(parent)
{
    initialize();
}

SelectMimeTypeComboBox::~SelectMimeTypeComboBox()
{
}

void SelectMimeTypeComboBox::initialize()
{
    //TODO verify
    addItem(i18n("JPEG"), QLatin1String("image/jpeg"));
    addItem(i18n("TIFF"), QLatin1String("image/tiff"));
    addItem(i18n("PNG"), QLatin1String("image/png"));
    addItem(i18n("BMP"), QLatin1String("image/bmp"));
}

QString SelectMimeTypeComboBox::code() const
{
    return QString::fromLatin1("\"%1\"").arg(itemData(currentIndex()).toString());
}

void SelectMimeTypeComboBox::setCode(const QString &code, const QString &name, QString &error)
{
    const int index = findData(code);
    if (index != -1) {
        setCurrentIndex(index);
    } else {
        AutoCreateScriptUtil::comboboxItemNotFound(code, name, error);
        setCurrentIndex(0);
    }
}


#include "selectmimetypecombobox.moc"
