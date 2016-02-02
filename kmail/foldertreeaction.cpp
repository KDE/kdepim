/*
  Copyright (C) 2016 eyeOS S.L.U., a Telefonica company, sales@eyeos.com

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

#include "foldertreeaction.h"
#include <KLocalizedString>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <kpushbutton.h>

FolderTreeAction::FolderTreeAction(QWidget *parent)
    : QWidget(parent)
{
    initializeWidgets();
}

FolderTreeAction::~FolderTreeAction()
{

}

void FolderTreeAction::initializeWidgets()
{
    QHBoxLayout *hboxLayout = new QHBoxLayout;
    setLayout(hboxLayout);
    hboxLayout->setMargin(18);
    hboxLayout->setSpacing(10);
    KPushButton *newMessage = new KPushButton(i18n("New Message"), this);
    newMessage->setDefault(true);
    connect(newMessage, SIGNAL(clicked()), this, SIGNAL(newMessage()));
    hboxLayout->addWidget(newMessage);
    KPushButton *checkMail = new KPushButton(this);
    checkMail->setDefault(true);
    checkMail->setIcon(KIcon(QLatin1String("mail-receive")));
    connect(checkMail, SIGNAL(clicked()), this, SIGNAL(checkMail()));
    hboxLayout->addWidget(checkMail);
}

