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

#include "mergecontactloseinformationwarning.h"
#include <KLocalizedString>
#include <QAction>

using namespace KABMergeContacts;
MergeContactLoseInformationWarning::MergeContactLoseInformationWarning(QWidget *parent)
    : KMessageWidget(parent)
{
    setVisible(false);
    setCloseButtonVisible(false);
    setMessageType(Information);
    setWordWrap(true);

    setText(i18n("Some information can be lost. Do you want to continue, or customize what you want to merge?"));

    QAction *action = new QAction(i18n("Customize"), this);
    action->setObjectName(QStringLiteral("customize"));
    connect(action, &QAction::triggered, this, &MergeContactLoseInformationWarning::slotCustomizeMerge);
    addAction(action);

    action = new QAction(i18n("Automatic Merging"), this);
    action->setObjectName(QStringLiteral("automatic"));
    connect(action, &QAction::triggered, this, &MergeContactLoseInformationWarning::slotAutomaticMerging);
    addAction(action);
}

MergeContactLoseInformationWarning::~MergeContactLoseInformationWarning()
{

}

void MergeContactLoseInformationWarning::slotCustomizeMerge()
{
    animatedHide();
    Q_EMIT customizeMergingContacts();
}

void MergeContactLoseInformationWarning::slotAutomaticMerging()
{
    animatedHide();
    Q_EMIT continueMerging();
}
