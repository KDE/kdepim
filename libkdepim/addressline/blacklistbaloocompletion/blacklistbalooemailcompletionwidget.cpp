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

#include "blacklistbalooemailcompletionwidget.h"
#include "blacklistbalooemaillist.h"
#include "blacklistbalooemailsearchjob.h"
#include "blacklistbalooemailutil.h"

#include <KLocalizedString>
#include <KLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>
#include <KConfigGroup>
#include <KSharedConfig>

using namespace KPIM;
BlackListBalooEmailCompletionWidget::BlackListBalooEmailCompletionWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QHBoxLayout *searchLayout = new QHBoxLayout;
    mainLayout->addLayout(searchLayout);

    QLabel *lab = new QLabel(i18n("Search email:"));
    lab->setObjectName(QLatin1String("search_label"));
    searchLayout->addWidget(lab);

    mSearchLineEdit = new KLineEdit;
    mSearchLineEdit->setFocus();
    mSearchLineEdit->setClearButtonShown(true);
    mSearchLineEdit->setTrapReturnKey(true);
    mSearchLineEdit->setObjectName(QLatin1String("search_lineedit"));
    connect(mSearchLineEdit, &KLineEdit::returnPressed, this, &BlackListBalooEmailCompletionWidget::slotSearch);
    searchLayout->addWidget(mSearchLineEdit);

    mSearchButton = new QPushButton(i18n("Search"));
    mSearchButton->setObjectName(QLatin1String("search_button"));
    connect(mSearchButton, &QAbstractButton::clicked, this, &BlackListBalooEmailCompletionWidget::slotSearch);
    mSearchButton->setEnabled(false);
    searchLayout->addWidget(mSearchButton);

    mEmailList = new BlackListBalooEmailList;
    mEmailList->setObjectName(QLatin1String("email_list"));
    mainLayout->addWidget(mEmailList);

    QHBoxLayout *selectElementLayout = new QHBoxLayout;
    mainLayout->addLayout(selectElementLayout);
    mSelectButton = new QPushButton(i18n("&Select"), this);
    mSelectButton->setObjectName(QLatin1String("select_email"));
    connect(mSelectButton, &QAbstractButton::clicked, this, &BlackListBalooEmailCompletionWidget::slotSelectEmails);
    selectElementLayout->addWidget(mSelectButton);

    mUnselectButton = new QPushButton(i18n("&Unselect"), this);
    mUnselectButton->setObjectName(QLatin1String("unselect_email"));
    connect(mUnselectButton, &QAbstractButton::clicked, this, &BlackListBalooEmailCompletionWidget::slotUnselectEmails);
    selectElementLayout->addWidget(mUnselectButton);
    selectElementLayout->addStretch(1);

    connect(mSearchLineEdit, &QLineEdit::textChanged, this, &BlackListBalooEmailCompletionWidget::slotSearchLineEditChanged);

    QHBoxLayout *excludeDomainLayout = new QHBoxLayout;
    excludeDomainLayout->setMargin(0);
    mainLayout->addLayout(excludeDomainLayout);

    QLabel *excludeDomainLabel = new QLabel(i18n("Exclude domain name:"));
    excludeDomainLabel->setObjectName(QLatin1String("domain_label"));
    excludeDomainLayout->addWidget(excludeDomainLabel);

    mExcludeDomainLineEdit = new KLineEdit;
    excludeDomainLayout->addWidget(mExcludeDomainLineEdit);
    mExcludeDomainLineEdit->setObjectName(QLatin1String("domain_lineedit"));
    mExcludeDomainLineEdit->setClearButtonShown(true);
    mExcludeDomainLineEdit->setTrapReturnKey(true);
    mExcludeDomainLineEdit->setClickMessage(i18n("Separate domain with \',\'"));
    connect(mEmailList, &QListWidget::itemSelectionChanged, this, &BlackListBalooEmailCompletionWidget::slotSelectionChanged);
    load();
}

BlackListBalooEmailCompletionWidget::~BlackListBalooEmailCompletionWidget()
{

}

void BlackListBalooEmailCompletionWidget::slotSelectionChanged()
{
    mSelectButton->setEnabled(!mEmailList->selectedItems().isEmpty());
    mUnselectButton->setEnabled(!mEmailList->selectedItems().isEmpty());
}

void BlackListBalooEmailCompletionWidget::load()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QLatin1String("kpimbalooblacklist"));
    KConfigGroup group(config, "AddressLineEdit");
    const QStringList lst = group.readEntry("ExcludeDomain", QStringList());
    mExcludeDomainLineEdit->setText(lst.join(QLatin1String(",")));
}

void BlackListBalooEmailCompletionWidget::slotUnselectEmails()
{
    Q_FOREACH (QListWidgetItem *item, mEmailList->selectedItems()) {
        item->setCheckState(Qt::Unchecked);
    }
}

void BlackListBalooEmailCompletionWidget::slotSelectEmails()
{
    Q_FOREACH (QListWidgetItem *item, mEmailList->selectedItems()) {
        item->setCheckState(Qt::Checked);
    }
}

void BlackListBalooEmailCompletionWidget::slotSearchLineEditChanged(const QString &text)
{
    mSearchButton->setEnabled(text.trimmed().count() > 2);
}

void BlackListBalooEmailCompletionWidget::slotSearch()
{
    const QString searchEmail = mSearchLineEdit->text().trimmed();
    if (searchEmail.length() > 2) {
        KPIM::BlackListBalooEmailSearchJob *job = new KPIM::BlackListBalooEmailSearchJob(this);
        job->setSearchEmail(searchEmail);
        connect(job, SIGNAL(emailsFound(QStringList)), mEmailList, SLOT(slotEmailFound(QStringList)));
        job->start();
    }
}

void BlackListBalooEmailCompletionWidget::setEmailBlackList(const QStringList &list)
{
    mEmailList->setEmailBlackList(list);
}

void BlackListBalooEmailCompletionWidget::save()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QLatin1String("kpimbalooblacklist"));
    KConfigGroup group(config, "AddressLineEdit");
    const QHash<QString, bool> result = mEmailList->blackListItemChanged();
    if (!result.isEmpty()) {
        QStringList blackList = group.readEntry("BalooBackList", QStringList());
        KPIM::BlackListBalooEmailUtil util;
        util.initialBlackList(blackList);
        util.newBlackList(result);
        blackList = util.createNewBlackList();
        group.writeEntry("BalooBackList", blackList);
    }
    group.writeEntry("ExcludeDomain", mExcludeDomainLineEdit->text().split(QLatin1String(",")));
    group.sync();
}
