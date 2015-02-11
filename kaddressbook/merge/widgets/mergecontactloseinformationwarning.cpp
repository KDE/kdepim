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

    //KF5 add i18n
    QAction *action = new QAction( QLatin1String( "Customize" ), this );
    action->setObjectName(QLatin1String("customize"));
    connect( action, SIGNAL(triggered(bool)), SLOT(slotCustomizeMerge()) );
    addAction( action );

    //KF5 add i18n
    action = new QAction( QLatin1String( "Automatic Merging" ), this );
    action->setObjectName(QLatin1String("automatic"));
    connect( action, SIGNAL(triggered(bool)), SLOT(slotAutomaticMerging()) );
    addAction( action );
}

MergeContactLoseInformationWarning::~MergeContactLoseInformationWarning()
{

}

void MergeContactLoseInformationWarning::slotCustomizeMerge()
{
    //TODO
}

void MergeContactLoseInformationWarning::slotAutomaticMerging()
{
    //TODO
}
