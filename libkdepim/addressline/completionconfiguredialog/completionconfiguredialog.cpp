/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "completionconfiguredialog.h"
#include <KLocalizedString>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <KConfigGroup>
#include <KGlobal>
#include <KSharedConfig>
#include <ldap/ldapclientsearch.h>
#include <addressline/completionorder/completionorderwidget.h>
#include <addressline/blacklistbaloocompletion/blacklistbalooemailcompletionwidget.h>
#include <addressline/recentaddress/recentaddresswidget.h>

using namespace KPIM;
CompletionConfigureDialog::CompletionConfigureDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Configure completion"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    mTabWidget = new QTabWidget;
    mTabWidget->setObjectName(QStringLiteral("tabwidget"));
    mainLayout->addWidget(mTabWidget);

    mCompletionOrderWidget = new KPIM::CompletionOrderWidget();
    mCompletionOrderWidget->setObjectName(QLatin1String("completionorder_widget"));

    mRecentaddressWidget = new KPIM::RecentAddressWidget;
    mRecentaddressWidget->setObjectName(QStringLiteral("recentaddress_widget"));

    mBlackListBalooWidget = new KPIM::BlackListBalooEmailCompletionWidget;
    mBlackListBalooWidget->setObjectName(QStringLiteral("blacklistbaloo_widget"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->setObjectName(QStringLiteral("buttonbox"));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotSave()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(buttonBox);

    mainLayout->addWidget(buttonBox);
    readConfig();
}

CompletionConfigureDialog::~CompletionConfigureDialog()
{
    writeConfig();
}

void CompletionConfigureDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "CompletionConfigureDialog");
    const QSize size = group.readEntry("Size", QSize(600, 400));
    if (size.isValid()) {
        resize(size);
    }
}

void CompletionConfigureDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "CompletionConfigureDialog");
    group.writeEntry("Size", size());
    group.sync();
}

void CompletionConfigureDialog::setRecentAddresses(const QStringList &lst)
{
    mRecentaddressWidget->setAddresses( lst );
}

void CompletionConfigureDialog::setLdapClientSearch(KLDAP::LdapClientSearch *ldapSearch)
{
    mCompletionOrderWidget->setLdapClientSearch(ldapSearch);
}

void CompletionConfigureDialog::load()
{
    mCompletionOrderWidget->loadCompletionItems();
}

void CompletionConfigureDialog::slotSave()
{
    mBlackListBalooWidget->save();
    if (mRecentaddressWidget->wasChanged()) {
        //TODO
    }
    //TODO
}

void CompletionConfigureDialog::setEmailBlackList(const QStringList &lst)
{
    mBlackListBalooWidget->setEmailBlackList(lst);
}
