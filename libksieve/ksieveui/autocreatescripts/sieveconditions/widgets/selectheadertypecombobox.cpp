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

#include "selectheadertypecombobox.h"

#include <KLocale>

using namespace KSieveUi;

SelectHeaderTypeComboBox::SelectHeaderTypeComboBox(QWidget *parent)
    : KComboBox(parent)
{
    setEditable(true);
    //TODO add completion
    initialize();
}

SelectHeaderTypeComboBox::~SelectHeaderTypeComboBox()
{
}

void SelectHeaderTypeComboBox::initialize()
{
    addItem(i18n("From"), QLatin1String("from"));
    addItem(i18n("To"), QLatin1String("to"));
    addItem(i18n("Cc"), QLatin1String("cc"));
    addItem(i18n("Bcc"), QLatin1String("bcc"));
    addItem(i18n("Sender"), QLatin1String("sender"));
    addItem(i18n("Sender-From"), QLatin1String("sender-from"));
    addItem(i18n("Sender-To"), QLatin1String("sender-to"));
}

QString SelectHeaderTypeComboBox::code() const
{
    QString str = itemData(currentIndex()).toString();
    if (str.isEmpty()) {
        str = currentText();
    }
    return itemData(currentIndex()).toString();
}

#include "selectheadertypecombobox.moc"
