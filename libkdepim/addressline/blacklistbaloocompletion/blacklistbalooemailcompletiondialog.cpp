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
#include "blacklistbalooemailcompletionwidget.h"
#include <KLocalizedString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
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
    mBlackListWidget = new BlackListBalooEmailCompletionWidget(this);
    setMainWidget(mBlackListWidget);

    connect(this, SIGNAL(okClicked()), this, SLOT(slotSave()));
    readConfig();
}

BlackListBalooEmailCompletionDialog::~BlackListBalooEmailCompletionDialog()
{
    writeConfig();
}

void BlackListBalooEmailCompletionDialog::setEmailBlackList(const QStringList &list)
{
    mBlackListWidget->setEmailBlackList(list);
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
    mBlackListWidget->save();
    accept();
}

