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
#include "sendlaterinfo.h"

#include <KLocale>
#include <KComboBox>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QDateTimeEdit>

SendLaterDialog::SendLaterDialog(SendLaterInfo *info, QWidget *parent)
    : KDialog(parent),
      mInfo(info)
{
    setCaption( i18n("Send Later") );
    setWindowIcon( KIcon( QLatin1String("kmail") ) );
    setButtons( User1|User2|Cancel );
    setButtonText( User1, i18n("Send Later"));
    setButtonText( User2, i18n("Send Now"));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSendLater()));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSendNow()));
    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    QHBoxLayout *hbox = new QHBoxLayout;
    lay->addLayout(hbox);

    QLabel *lab = new QLabel(i18n("Send at:"));

    mDateTime = new QDateTimeEdit;
    mDateTime->setMinimumDateTime(QDateTime::currentDateTime());
    hbox->addWidget(lab);
    hbox->addWidget(mDateTime);

    mRecursive = new QCheckBox(i18n("Recursive"));
    connect(mRecursive, SIGNAL(clicked(bool)), this, SLOT(slotRecursiveClicked(bool)));
    lay->addWidget(mRecursive);

    hbox = new QHBoxLayout;
    lay->addLayout(hbox);

    lab = new QLabel(i18n("Each:"));
    hbox->addWidget(lab);

    mRecursiveValue = new QSpinBox;
    mRecursiveValue->setMinimum(1);
    hbox->addWidget(mRecursiveValue);

    mRecursiveComboBox = new KComboBox;
    QStringList unitsList;
    unitsList<<i18n("Days");
    unitsList<<i18n("Weeks");
    unitsList<<i18n("Months");
    //Years ?
    mRecursiveComboBox->addItems(unitsList);

    hbox->addWidget(mRecursiveComboBox);

    w->setLayout(lay);
    setMainWidget(w);
    readConfig();
    if (info)
        load(info);
    slotRecursiveClicked(false);
}

SendLaterDialog::~SendLaterDialog()
{
    writeConfig();
}

void SendLaterDialog::slotRecursiveClicked(bool clicked)
{
    mRecursiveValue->setEnabled(clicked);
    mRecursiveComboBox->setEnabled(clicked);
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

void SendLaterDialog::load(SendLaterInfo *info)
{
    mDateTime->setDateTime(info->dateTime());
    mRecursive->setChecked(info->isRecursive());
    mRecursiveValue->setValue(info->recursiveEachValue());
    mRecursiveComboBox->setCurrentIndex((int)info->recursiveUnit());
}

SendLaterInfo* SendLaterDialog::info()
{
    if (!mInfo) {
        mInfo = new SendLaterInfo();
        mInfo->setItemId(-1);
    }
    mInfo->setRecursive(mRecursive->isChecked());
    mInfo->setRecursiveEachValue(mRecursiveValue->value());
    mInfo->setRecursiveUnit((SendLaterInfo::RecursiveUnit)mRecursiveComboBox->currentIndex());
    mInfo->setDateTime(mDateTime->dateTime());
    return mInfo;
}


void SendLaterDialog::slotSendLater()
{
    //TODO
    accept();
}

void SendLaterDialog::slotSendNow()
{
    accept();
    //TODO
}


#include "sendlaterdialog.moc"
