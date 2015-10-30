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
#include "messagelistsettings.h"
#include <KLocalizedString>
#include <QAction>
using namespace MessageList::Core;

QuickSearchWarning::QuickSearchWarning(QWidget *parent)
    : KMessageWidget(parent)
{
    setVisible(false);
    setCloseButtonVisible(true);
    setMessageType(Warning);
    setWordWrap(true);
    setText(i18n("The words less than 3 letters are ignored."));
    QAction *action = new QAction(i18n("Do not show again"), this);
    action->setObjectName(QStringLiteral("donotshowagain"));
    connect(action, &QAction::triggered, this, &QuickSearchWarning::slotDoNotRememberIt);
    addAction(action);

}

QuickSearchWarning::~QuickSearchWarning()
{

}

void QuickSearchWarning::setSearchText(const QString &text)
{
    if (!MessageList::MessageListSettings::quickSearchWarningDoNotShowAgain()) {
        const QStringList lstText = text.split(QLatin1Char(' '), QString::SkipEmptyParts);
        bool foundLessThanThreeCharacters = false;
        Q_FOREACH (const QString &text, lstText) {
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
}

void QuickSearchWarning::slotDoNotRememberIt()
{
    MessageList::MessageListSettings::setQuickSearchWarningDoNotShowAgain(true);
    animatedHide();
}
