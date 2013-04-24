/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "sendlaterdialog.h"

#include <KLocale>

SendLaterDialog::SendLaterDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n("Send Later") );
    setButtons( User1|User2|Cancel );
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSendLater()));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSendNow()));
    QWidget *w = new QWidget;
    setMainWidget(w);
    readConfig();
}

SendLaterDialog::~SendLaterDialog()
{
    writeConfig();
}

void SendLaterDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "SendLaterDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize() );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    } else {
        resize( 800,600);
    }
}

void SendLaterDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "SendLaterDialog" );
    group.writeEntry( "Size", size() );
}

void SendLaterDialog::slotSendLater()
{
    //TODO
}

void SendLaterDialog::slotSendNow()
{
    //TODO
}

#include "sendlaterdialog.moc"
