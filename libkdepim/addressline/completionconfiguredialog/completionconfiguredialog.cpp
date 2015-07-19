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
    mCompletionOrderWidget->setObjectName(QStringLiteral("completionorder_widget"));
    mTabWidget->addTab(mCompletionOrderWidget, i18n("Completion Order"));

    mRecentaddressWidget = new KPIM::RecentAddressWidget;
    mRecentaddressWidget->setObjectName(QStringLiteral("recentaddress_widget"));
    mTabWidget->addTab(mRecentaddressWidget, i18n("Recent Address"));

    mBlackListBalooWidget = new KPIM::BlackListBalooEmailCompletionWidget;
    mBlackListBalooWidget->setObjectName(QStringLiteral("blacklistbaloo_widget"));
    mTabWidget->addTab(mBlackListBalooWidget, i18n("Blacklist Email Address"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->setObjectName(QStringLiteral("buttonbox"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &CompletionConfigureDialog::slotSave);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
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
    mRecentaddressWidget->setAddresses(lst);
}

void CompletionConfigureDialog::setLdapClientSearch(KLDAP::LdapClientSearch *ldapSearch)
{
    mCompletionOrderWidget->setLdapClientSearch(ldapSearch);
}

void CompletionConfigureDialog::load()
{
    mCompletionOrderWidget->loadCompletionItems();
    mBlackListBalooWidget->load();
}

bool CompletionConfigureDialog::recentAddressWasChanged() const
{
    return mRecentaddressWidget->wasChanged();
}

void CompletionConfigureDialog::storeAddresses(KConfig *config)
{
    mRecentaddressWidget->storeAddresses(config);
}

void CompletionConfigureDialog::slotSave()
{
    mBlackListBalooWidget->save();
    mCompletionOrderWidget->save();
    accept();
}

void CompletionConfigureDialog::setEmailBlackList(const QStringList &lst)
{
    mBlackListBalooWidget->setEmailBlackList(lst);
}
