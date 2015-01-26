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
#include <KLocalizedString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <KLineEdit>
#include <KPushButton>

using namespace KPIM;

BlackListBalooEmailCompletionDialog::BlackListBalooEmailCompletionDialog(QWidget *parent)
    : KDialog(parent)
{
    //Add i18n in kf5
    setCaption( QLatin1String( "Blacklist Baloo Completion" ) );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );

    QWidget *mainWidget = new QWidget( this );
    setMainWidget(mainWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainWidget->setLayout(mainLayout);

    QHBoxLayout *searchLayout = new QHBoxLayout;
    mainLayout->addLayout(searchLayout);

    //Add i18n in kf5
    QLabel *lab = new QLabel(QLatin1String("Search email:"));
    lab->setObjectName(QLatin1String("search_label"));
    searchLayout->addWidget(lab);

    mSearchLineEdit = new KLineEdit;
    mSearchLineEdit->setClearButtonShown(true);
    mSearchLineEdit->setTrapReturnKey(true);
    mSearchLineEdit->setObjectName(QLatin1String("search_lineedit"));
    searchLayout->addWidget(mSearchLineEdit);

    //Add i18n in kf5
    mSearchButton = new KPushButton(QLatin1String("Search"));
    mSearchButton->setObjectName(QLatin1String("search_button"));
    mSearchButton->setEnabled(false);
    searchLayout->addWidget(mSearchButton);

    mEmailList = new BlackListBalooEmailList;
    mEmailList->setObjectName(QLatin1String("email_list"));
    mainLayout->addWidget(mEmailList);

    connect(mSearchLineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotSearchLineEditChanged(QString)));
    connect(this, SIGNAL(okClicked()), this, SLOT(slotSave()));
    readConfig();
}

BlackListBalooEmailCompletionDialog::~BlackListBalooEmailCompletionDialog()
{
    writeConfig();
}

void BlackListBalooEmailCompletionDialog::slotSearchLineEditChanged(const QString &text)
{
    mSearchButton->setEnabled(text.trimmed().count() > 2);
}

void BlackListBalooEmailCompletionDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "BlackListBalooEmailCompletionDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void BlackListBalooEmailCompletionDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "BlackListBalooEmailCompletionDialog" );
    group.writeEntry( "Size", size() );
}

void BlackListBalooEmailCompletionDialog::slotSave()
{
    //TODO
}

void BlackListBalooEmailCompletionDialog::slotSearch()
{
    const QString searchEmail = mSearchLineEdit->text().trimmed();
    KPIM::BlackListBalooEmailSearchJob *job = new KPIM::BlackListBalooEmailSearchJob(this);
    connect(job, SIGNAL(emailsFound(QStringList)), mEmailList, SLOT(slotEmailFound(QStringList)));
}

