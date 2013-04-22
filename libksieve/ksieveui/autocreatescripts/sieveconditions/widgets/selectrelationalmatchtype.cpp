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


#include "selectrelationalmatchtype.h"

#include <KLocale>

using namespace KSieveUi;

SelectRelationalMatchType::SelectRelationalMatchType(QWidget *parent)
    : KComboBox(parent)
{
    initialize();
}

SelectRelationalMatchType::~SelectRelationalMatchType()
{

}

QString SelectRelationalMatchType::code() const
{
    return QString::fromLatin1("\"%1\"").arg(itemData(currentIndex()).toString());
}

void SelectRelationalMatchType::initialize()
{
    addItem(i18n("Greater than"), QLatin1String("gt"));
    addItem(i18n("Greater than or equal"), QLatin1String("ge"));
    addItem(i18n("Less than"), QLatin1String("lt"));
    addItem(i18n("Less than or equal"), QLatin1String("le"));
    addItem(i18n("Equal to"), QLatin1String("eq"));
    addItem(i18n("Not equal to"), QLatin1String("ne"));
}

#include "selectrelationalmatchtype.moc"
