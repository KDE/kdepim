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

    setText(QLatin1String("Some merge can lose information. Do you want to continue or customize what merged ?"));

    //KF5 add i18n
    QAction *action = new QAction(QLatin1String("Customize"), this);
    action->setObjectName(QLatin1String("customize"));
    connect(action, &QAction::triggered, this, &MergeContactLoseInformationWarning::slotCustomizeMerge);
    addAction(action);

    //KF5 add i18n
    action = new QAction(QLatin1String("Automatic Merging"), this);
    action->setObjectName(QLatin1String("automatic"));
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
