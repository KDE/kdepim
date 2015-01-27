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

#include "blacklistbalooemailcompletiondialog.h"
#include "blacklistbalooemailsearchjob.h"
#include "blacklistbalooemaillist.h"
#include "blacklistbalooemailutil.h"
#include <KLocalizedString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <KLineEdit>
#include <QPushButton>
#include <KSharedConfig>

using namespace KPIM;

BlackListBalooEmailCompletionDialog::BlackListBalooEmailCompletionDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Blacklist Baloo Completion" ) );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );

    QWidget *mainWidget = new QWidget( this );
    setMainWidget(mainWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainWidget->setLayout(mainLayout);

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
    connect(mSearchLineEdit, SIGNAL(returnPressed()), this, SLOT(slotSearch()));
    searchLayout->addWidget(mSearchLineEdit);

    mSearchButton = new QPushButton(i18n("Search"));
    mSearchButton->setObjectName(QLatin1String("search_button"));
    connect(mSearchButton, SIGNAL(clicked()), this, SLOT(slotSearch()));
    mSearchButton->setEnabled(false);
    searchLayout->addWidget(mSearchButton);

    mEmailList = new BlackListBalooEmailList;
    mEmailList->setObjectName(QLatin1String("email_list"));
    mainLayout->addWidget(mEmailList);


    QHBoxLayout *selectElementLayout = new QHBoxLayout;
    mainLayout->addLayout(selectElementLayout);
    QPushButton *button = new QPushButton(i18n("&Select"), this);
    button->setObjectName(QLatin1String("select_email"));
    connect(button, SIGNAL(clicked(bool)), this, SLOT(slotSelectEmails()));
    selectElementLayout->addWidget(button);

    button = new QPushButton(i18n("&Unselect"), this);
    button->setObjectName(QLatin1String("unselect_email"));
    connect(button, SIGNAL(clicked(bool)), this, SLOT(slotUnselectEmails()));
    selectElementLayout->addWidget(button);
    selectElementLayout->addStretch(1);


    connect(mSearchLineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotSearchLineEditChanged(QString)));
    connect(this, SIGNAL(okClicked()), this, SLOT(slotSave()));
    readConfig();
}

BlackListBalooEmailCompletionDialog::~BlackListBalooEmailCompletionDialog()
{
    writeConfig();
}

void BlackListBalooEmailCompletionDialog::slotUnselectEmails()
{
    Q_FOREACH(QListWidgetItem *item, mEmailList->selectedItems()) {
        item->setCheckState(Qt::Unchecked);
    }
}

void BlackListBalooEmailCompletionDialog::slotSelectEmails()
{
    Q_FOREACH(QListWidgetItem *item, mEmailList->selectedItems()) {
        item->setCheckState(Qt::Checked);
    }
}

void BlackListBalooEmailCompletionDialog::setEmailBlackList(const QStringList &list)
{
    mEmailList->setEmailBlackList(list);
}

void BlackListBalooEmailCompletionDialog::slotSearchLineEditChanged(const QString &text)
{
    mSearchButton->setEnabled(text.trimmed().count() > 2);
}

void BlackListBalooEmailCompletionDialog::readConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "BlackListBalooEmailCompletionDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void BlackListBalooEmailCompletionDialog::writeConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "BlackListBalooEmailCompletionDialog" );
    group.writeEntry( "Size", size() );
}

void BlackListBalooEmailCompletionDialog::slotSave()
{
    const QHash<QString, bool> result = mEmailList->blackListItemChanged();
    if (!result.isEmpty()) {
        KConfigGroup group( KSharedConfig::openConfig(), "AddressLineEdit" );
        QStringList blackList = group.readEntry( "BalooBackList", QStringList() );
        KPIM::BlackListBalooEmailUtil util;
        util.initialBlackList(blackList);
        util.newBlackList(result);
        blackList = util.createNewBlackList();
        group.writeEntry( "BalooBackList", blackList );
        group.sync();
    }
    accept();
}

void BlackListBalooEmailCompletionDialog::slotSearch()
{
    const QString searchEmail = mSearchLineEdit->text().trimmed();
    if (searchEmail.length() > 2 ) {
        KPIM::BlackListBalooEmailSearchJob *job = new KPIM::BlackListBalooEmailSearchJob(this);
        job->setSearchEmail(searchEmail);
        connect(job, SIGNAL(emailsFound(QStringList)), mEmailList, SLOT(slotEmailFound(QStringList)));
        job->start();
    }
}

