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
#include <KComboBox>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>

SendLaterDialog::SendLaterDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n("Send Later") );
    setWindowIcon( KIcon( "kmail" ) );
    setButtons( User1|User2|Cancel );
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSendLater()));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSendNow()));
    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    mRecursive = new QCheckBox(i18n("Recursive"));
    connect(mRecursive, SIGNAL(clicked(bool)), this, SLOT(slotRecursiveClicked(bool)));
    lay->addWidget(mRecursive);

    QHBoxLayout *hbox = new QHBoxLayout;
    lay->addLayout(hbox);

    QLabel *lab = new QLabel(i18n("Each:"));
    hbox->addWidget(lab);

    mRecursiveValue = new QSpinBox;
    hbox->addWidget(mRecursiveValue);

    mRecursiveComboBox = new KComboBox;
    QStringList unitsList;
    unitsList<<i18n("Days");
    unitsList<<i18n("Weeks");
    unitsList<<i18n("Months");
    mRecursiveComboBox->addItems(unitsList);

    hbox->addWidget(mRecursiveComboBox);

    setLayout(lay);
    setMainWidget(w);
    readConfig();
}

SendLaterDialog::~SendLaterDialog()
{
    writeConfig();
}

void SendLaterDialog::slotRecursiveClicked(bool)
{

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
