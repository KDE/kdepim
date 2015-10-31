/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "quicksearchwarning.h"
using namespace MessageList::Core;

QuickSearchWarning::QuickSearchWarning(QWidget *parent)
    : KMessageWidget(parent)
{
    setVisible(false);
    setCloseButtonVisible(true);
    setMessageType(Warning);
    setWordWrap(true);
    //KF5 add i18n
    setText(QLatin1String("The words less than 3 letters are ignored."));
}

QuickSearchWarning::~QuickSearchWarning()
{

}

void QuickSearchWarning::setSearchText(const QString &text)
{
    const QStringList lstText = text.split(QLatin1Char(' '), QString::SkipEmptyParts);
    bool foundLessThanThreeCharacters = false;
    Q_FOREACH(const QString &text, lstText) {
        if (text.trimmed().size() < 3) {
            foundLessThanThreeCharacters = true;
            break;
        }
    }
    if (foundLessThanThreeCharacters) {
        animatedShow();
    } else {
        animatedHide();
    }
}

