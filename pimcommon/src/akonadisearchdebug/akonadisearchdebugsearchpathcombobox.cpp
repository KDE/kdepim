/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "akonadisearchdebugsearchpathcombobox.h"
#include <QStandardPaths>

using namespace PimCommon;
AkonadiSearchDebugSearchPathComboBox::AkonadiSearchDebugSearchPathComboBox(QWidget *parent)
    : QComboBox(parent)
{
    initialize();
}

AkonadiSearchDebugSearchPathComboBox::~AkonadiSearchDebugSearchPathComboBox()
{

}

QString AkonadiSearchDebugSearchPathComboBox::searchPath() const
{
    const int currentPathIndex = currentIndex();
    if (currentPathIndex > -1) {
        const QString value = pathFromEnum(static_cast<PimCommon::AkonadiSearchDebugSearchPathComboBox::SearchType>(itemData(currentPathIndex).toInt()));
        return value;
    } else {
        return QString();
    }
}

void AkonadiSearchDebugSearchPathComboBox::initialize()
{
    addItem(QStringLiteral("Contacts"), Contacts);
    addItem(QStringLiteral("ContactCompleter"), ContactCompleter);
    addItem(QStringLiteral("Email"), Emails);
    addItem(QStringLiteral("Notes"), Notes);
    addItem(QStringLiteral("Calendars"), Calendars);
}

QString AkonadiSearchDebugSearchPathComboBox::pathFromEnum(SearchType type) const
{
    const QString xdgpath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/');
    switch (type) {
    case Contacts:
        return QString(xdgpath + QLatin1String("baloo/contacts/"));
    case ContactCompleter:
        return QString(xdgpath + QLatin1String("baloo/emailContacts/"));
    case Emails:
        return QString(xdgpath + QLatin1String("baloo/email/"));
    case Notes:
        return QString(xdgpath + QLatin1String("baloo/notes/"));
    case Calendars:
        return QString(xdgpath + QLatin1String("baloo/calendars/"));
    }
    return QString();
}

void AkonadiSearchDebugSearchPathComboBox::setSearchType(AkonadiSearchDebugSearchPathComboBox::SearchType type)
{
    const int indexType = findData(type);
    if (indexType >= 0) {
        setCurrentIndex(indexType);
    }
}
