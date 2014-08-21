/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "storageserviceaccountinfodialog.h"
#include "pimcommon/storageservice/storageserviceabstract.h"

#include <KLocalizedString>

#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <KFormat>

StorageServiceAccountInfoDialog::StorageServiceAccountInfoDialog(const QString &serviceName, const PimCommon::AccountInfo &accountInfo, QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18n("Account Info"));
    setButtons(Close);

    QGroupBox *grp = new QGroupBox(serviceName);
    setMainWidget(grp);

    QVBoxLayout *vbox = new QVBoxLayout;
    grp->setLayout(vbox);
    if (accountInfo.isValid()) {
        if (accountInfo.accountSize >= 0) {
            vbox->addWidget(new QLabel(i18n("Size: %1", KFormat().formatByteSize(accountInfo.accountSize, 1))));
        }
        if (accountInfo.quota >= 0) {
            vbox->addWidget(new QLabel(i18n("Quota: %1", KFormat().formatByteSize(accountInfo.quota, 1))));
        }
        if (accountInfo.shared >= 0) {
            vbox->addWidget(new QLabel(i18n("Shared: %1", KFormat().formatByteSize(accountInfo.shared, 1))));
        }
    } else {
        QLabel *lab = new QLabel(i18n("Unable to get account information."));
        QFont font = lab->font();
        font.setBold(true);
        lab->setFont(font);
        vbox->addWidget(lab);
    }
}

StorageServiceAccountInfoDialog::~StorageServiceAccountInfoDialog()
{

}

