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
#include <KPushButton>
#include <QBoxLayout>
#include <QLabel>
#include <KConfigGroup>
using namespace KPIM;
BlackListBalooEmailCompletionWidget::BlackListBalooEmailCompletionWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QHBoxLayout *searchLayout = new QHBoxLayout;
    mainLayout->addLayout(searchLayout);

    //Add i18n in kf5
    QLabel *lab = new QLabel(QLatin1String("Search email:"));
    lab->setObjectName(QLatin1String("search_label"));
    searchLayout->addWidget(lab);

    mSearchLineEdit = new KLineEdit;
    mSearchLineEdit->setFocus();
    mSearchLineEdit->setClearButtonShown(true);
    mSearchLineEdit->setTrapReturnKey(true);
    mSearchLineEdit->setObjectName(QLatin1String("search_lineedit"));
    connect(mSearchLineEdit, SIGNAL(returnPressed()), this, SLOT(slotSearch()));
    searchLayout->addWidget(mSearchLineEdit);

    //Add i18n in kf5
    mSearchButton = new KPushButton(QLatin1String("Search"));
    mSearchButton->setObjectName(QLatin1String("search_button"));
    connect(mSearchButton, SIGNAL(clicked()), this, SLOT(slotSearch()));
    mSearchButton->setEnabled(false);
    searchLayout->addWidget(mSearchButton);

    mEmailList = new BlackListBalooEmailList;
    mEmailList->setObjectName(QLatin1String("email_list"));
    mainLayout->addWidget(mEmailList);


    QHBoxLayout *selectElementLayout = new QHBoxLayout;
    mainLayout->addLayout(selectElementLayout);
    //Add i18n in kf5
    KPushButton *button = new KPushButton(QLatin1String("&Select"), this);
    button->setObjectName(QLatin1String("select_email"));
    connect(button, SIGNAL(clicked(bool)), this, SLOT(slotSelectEmails()));
    selectElementLayout->addWidget(button);

    //Add i18n in kf5
    button = new KPushButton(QLatin1String("&Unselect"), this);
    button->setObjectName(QLatin1String("unselect_email"));
    connect(button, SIGNAL(clicked(bool)), this, SLOT(slotUnselectEmails()));
    selectElementLayout->addWidget(button);
    selectElementLayout->addStretch(1);


    connect(mSearchLineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotSearchLineEditChanged(QString)));

    QHBoxLayout *excludeDomainLayout = new QHBoxLayout;
    excludeDomainLayout->setMargin(0);
    mainLayout->addLayout(excludeDomainLayout);

    //kf5 add i18n
    QLabel *excludeDomainLabel = new QLabel(QLatin1String("Exclude domain name:"));
    excludeDomainLabel->setObjectName(QLatin1String("domain_label"));
    excludeDomainLayout->addWidget(excludeDomainLabel);

    mExcludeDomainLineEdit = new KLineEdit;
    excludeDomainLayout->addWidget(mExcludeDomainLineEdit);
    mExcludeDomainLineEdit->setObjectName(QLatin1String("domain_lineedit"));
    mExcludeDomainLineEdit->setClearButtonShown(true);
    mExcludeDomainLineEdit->setTrapReturnKey(true);
    //kf5 add i18n
    mExcludeDomainLineEdit->setClickMessage(QLatin1String("Separate domain with \',\'"));
    load();
}

BlackListBalooEmailCompletionWidget::~BlackListBalooEmailCompletionWidget()
{

}

void BlackListBalooEmailCompletionWidget::load()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig( QLatin1String("kpimbalooblacklist") );
    KConfigGroup group( config, "AddressLineEdit" );
    const QStringList lst = group.readEntry("ExcludeDomain", QStringList());
    mExcludeDomainLineEdit->setText(lst.join(QLatin1String(",")));
}

void BlackListBalooEmailCompletionWidget::slotUnselectEmails()
{
    Q_FOREACH(QListWidgetItem *item, mEmailList->selectedItems()) {
        item->setCheckState(Qt::Unchecked);
    }
}

void BlackListBalooEmailCompletionWidget::slotSelectEmails()
{
    Q_FOREACH(QListWidgetItem *item, mEmailList->selectedItems()) {
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
    if (searchEmail.length() > 2 ) {
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
    KSharedConfig::Ptr config = KSharedConfig::openConfig( QLatin1String("kpimbalooblacklist") );
    KConfigGroup group( config, "AddressLineEdit" );
    const QHash<QString, bool> result = mEmailList->blackListItemChanged();
    if (!result.isEmpty()) {
        QStringList blackList = group.readEntry( "BalooBackList", QStringList() );
        KPIM::BlackListBalooEmailUtil util;
        util.initialBlackList(blackList);
        util.newBlackList(result);
        blackList = util.createNewBlackList();
        group.writeEntry( "BalooBackList", blackList );
    }
    group.writeEntry("ExcludeDomain", mExcludeDomainLineEdit->text().split(QLatin1String(",")));
    group.sync();
}
