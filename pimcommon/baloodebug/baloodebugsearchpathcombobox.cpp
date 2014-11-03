/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "baloodebugsearchpathcombobox.h"
#include <KGlobal>
#include <KStandardDirs>

#include <QDebug>

using namespace PimCommon;
BalooDebugSearchPathComboBox::BalooDebugSearchPathComboBox(QWidget *parent)
    : QComboBox(parent)
{
    initialize();
}

BalooDebugSearchPathComboBox::~BalooDebugSearchPathComboBox()
{

}

QString BalooDebugSearchPathComboBox::searchPath() const
{
    const int currentPathIndex = currentIndex();
    if (currentPathIndex > -1) {
        const QString value = pathFromEnum(static_cast<PimCommon::BalooDebugSearchPathComboBox::SearchType>(itemData(currentPathIndex).toInt()));
        return value;
    } else {
        return QString();
    }
}

void BalooDebugSearchPathComboBox::initialize()
{
    addItem(QLatin1String("Contacts"), Contacts);
    addItem(QLatin1String("ContactCompleter"), ContactCompleter);
    addItem(QLatin1String("Email"), Emails);
    addItem(QLatin1String("Notes"), Notes);
}

QString BalooDebugSearchPathComboBox::pathFromEnum(SearchType type) const
{
    const QString xdgpath = KGlobal::dirs()->localxdgdatadir();
    switch(type) {
    case Contacts:
        return QString(xdgpath + QLatin1String("baloo/contacts/"));
    case ContactCompleter:
        return QString(xdgpath + QLatin1String("baloo/emailContacts/"));
    case Emails:
        return QString(xdgpath + QLatin1String("baloo/email/"));
    case Notes:
        return QString(xdgpath + QLatin1String("baloo/notes/"));
    }
    return QString();
}

void BalooDebugSearchPathComboBox::setSearchType(BalooDebugSearchPathComboBox::SearchType type)
{
    const int indexType = findData(type);
    if (indexType >= 0) {
        setCurrentIndex(indexType);
    }
}
