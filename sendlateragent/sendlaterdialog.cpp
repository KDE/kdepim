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
#include "sendlatertimedatewidget.h"

#include <KLocale>
#include <KComboBox>
#include <KPushButton>
#include <KSeparator>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>

using namespace SendLater;

SendLaterDialog::SendLaterDialog(SendLater::SendLaterInfo *info, QWidget *parent)
    : KDialog(parent),
      mAction(SendLater),
      mInfo(info),
      mSendAtTime(0),
      mSendAtTimeLabel(0)
{
    setCaption( i18n("Send Later") );
    setWindowIcon( KIcon( QLatin1String("kmail") ) );
    if (info) {
        setButtons( Ok|Cancel );
        connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));
    } else {
        setButtons( User1|Cancel );
        setButtonText( User1, i18n("Send Later"));
        connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSendLater()));
    }

    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    QHBoxLayout *hbox = new QHBoxLayout;
    lay->addLayout(hbox);

    QLabel *lab = new QLabel(i18n("Send at:"));

    QDateTime t = QDateTime::currentDateTime();
    t = t.addSecs(60*60);

    mDateTime = new SendLaterTimeDateWidget;
    mDateTime->setDateTime(t);
    connect(mDateTime, SIGNAL(dateTimeChanged(QDateTime)), SLOT(slotDateTimeChanged(QDateTime)));
    hbox->addWidget(lab);
    hbox->addWidget(mDateTime);

    mRecurrence = new QCheckBox(i18n("Recurrence"));
    connect(mRecurrence, SIGNAL(clicked(bool)), this, SLOT(slotRecurrenceClicked(bool)));
    lay->addWidget(mRecurrence);

    hbox = new QHBoxLayout;
    lay->addLayout(hbox);

    lab = new QLabel(i18n("Each:"));
    hbox->addWidget(lab);

    mRecurrenceValue = new QSpinBox;
    mRecurrenceValue->setMinimum(1);
    hbox->addWidget(mRecurrenceValue);

    mRecurrenceComboBox = new KComboBox;
    QStringList unitsList;
    unitsList<<i18n("Days");
    unitsList<<i18n("Weeks");
    unitsList<<i18n("Months");
    //Years ?
    mRecurrenceComboBox->addItems(unitsList);

    hbox->addWidget(mRecurrenceComboBox);

    if (info) {
        mSendAtTimeLabel = new QLabel;
        hbox->addWidget(mSendAtTimeLabel);
    } else {
        mSendAtTime = new KPushButton;
        connect(mSendAtTime, SIGNAL(clicked()), SLOT(slotSendAtTime()));
        hbox->addWidget(mSendAtTime);
    }

    hbox = new QHBoxLayout;

    mSendIn30Minutes = new KPushButton(i18n("30 minutes later"));
    connect(mSendIn30Minutes, SIGNAL(clicked()), SLOT(slotSendIn30Minutes()));
    hbox->addWidget(mSendIn30Minutes);

    mSendIn1Hour = new KPushButton(i18n("1 hour later"));
    connect(mSendIn1Hour, SIGNAL(clicked()), SLOT(slotSendIn1Hour()));
    hbox->addWidget(mSendIn1Hour);

    mSendIn2Hours = new KPushButton(i18n("2 hours later"));
    connect(mSendIn2Hours, SIGNAL(clicked()), SLOT(slotSendIn2Hours()));
    hbox->addWidget(mSendIn2Hours);
    lay->addLayout(hbox);

    lay->addWidget(new KSeparator);

    w->setLayout(lay);
    setMainWidget(w);
    readConfig();
    slotRecurrenceClicked(false);
    //Add currentdate/time  + 1h
    slotDateTimeChanged(t);
    if (info)
        load(info);
}

SendLaterDialog::~SendLaterDialog()
{
    writeConfig();
}

void SendLaterDialog::slotSendIn2Hours()
{
    mSendDateTime = QDateTime::currentDateTime().addSecs(60*60*2);
    mAction = SendDeliveryAtTime;
    accept();
}

void SendLaterDialog::slotSendIn1Hour()
{
    mSendDateTime = QDateTime::currentDateTime().addSecs(60*60);
    mAction = SendDeliveryAtTime;
    accept();
}

void SendLaterDialog::slotSendIn30Minutes()
{
    mSendDateTime = QDateTime::currentDateTime().addSecs(60*30);
    mAction = SendDeliveryAtTime;
    accept();
}

void SendLaterDialog::slotRecurrenceClicked(bool clicked)
{
    mRecurrenceValue->setEnabled(clicked);
    mRecurrenceComboBox->setEnabled(clicked);
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

void SendLaterDialog::load(SendLater::SendLaterInfo *info)
{
    mDateTime->setDateTime(info->dateTime());
    const bool recurrence = info->isRecurrence();
    mRecurrence->setChecked(recurrence);
    slotRecurrenceClicked(recurrence);
    mRecurrenceValue->setValue(info->recurrenceEachValue());
    mRecurrenceComboBox->setCurrentIndex((int)info->recurrenceUnit());
}

SendLater::SendLaterInfo* SendLaterDialog::info()
{
    if (!mInfo) {
        mInfo = new SendLater::SendLaterInfo();
    }
    mInfo->setRecurrence(mRecurrence->isChecked());
    mInfo->setRecurrenceEachValue(mRecurrenceValue->value());
    mInfo->setRecurrenceUnit((SendLater::SendLaterInfo::RecurrenceUnit)mRecurrenceComboBox->currentIndex());
    if (mSendDateTime.isValid())
        mInfo->setDateTime(mSendDateTime);
    else
        mInfo->setDateTime(mDateTime->dateTime());
    return mInfo;
}

void SendLaterDialog::slotSendLater()
{
    mAction = SendLater;
    accept();
}

SendLaterDialog::SendLaterAction SendLaterDialog::action() const
{
    return mAction;
}

void SendLaterDialog::slotDateTimeChanged(const QDateTime &datetime)
{
    const QString str = i18n("Send around %1", KGlobal::locale()->formatDateTime(datetime));
    if (mSendAtTime)
        mSendAtTime->setText(str);
    else if (mSendAtTimeLabel)
        mSendAtTimeLabel->setText(str);
}

void SendLaterDialog::slotSendAtTime()
{
    mSendDateTime = mDateTime->dateTime();
    mAction = SendDeliveryAtTime;
    accept();
}

void SendLaterDialog::slotOkClicked()
{
    mSendDateTime = mDateTime->dateTime();
    mAction = SendDeliveryAtTime;
    accept();
}

#include "sendlaterdialog.moc"
