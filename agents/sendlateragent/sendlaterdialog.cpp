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
#include "ui_sendlaterwidget.h"

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
      mAction(Unknown),
      mInfo(info)
{
    setCaption( i18n("Send Later") );
    setWindowIcon( KIcon( QLatin1String("kmail") ) );

    QWidget *sendLaterWidget = new QWidget;
    mSendLaterWidget = new Ui::SendLaterWidget;
    mSendLaterWidget->setupUi( sendLaterWidget );

    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    w->setLayout(lay);

    setButtons( Ok|Cancel );

    if (!info) {
        setButtonText( Ok, i18n("Send Later"));
        mDelay = new QCheckBox(i18n("Delay"));
        mDelay->setChecked(false);
        slotDelay(false);
        connect(mDelay, SIGNAL(clicked(bool)), this, SLOT(slotDelay(bool)));
        lay->addWidget(mDelay);
    }

    connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));

    lay->addWidget(sendLaterWidget);

    QDateTime t = QDateTime::currentDateTime();
    t = t.addSecs(60*60);

    mSendLaterWidget->mDateTime->setDateTime(t);
    connect(mSendLaterWidget->mRecurrence, SIGNAL(clicked(bool)), this, SLOT(slotRecurrenceClicked(bool)));
    QStringList unitsList;
    unitsList<<i18n("Days");
    unitsList<<i18n("Weeks");
    unitsList<<i18n("Months");
    unitsList<<i18n("Years");
    mSendLaterWidget->mRecurrenceComboBox->addItems(unitsList);

    lay->addWidget(new KSeparator);

    w->setLayout(lay);
    setMainWidget(w);
    slotRecurrenceClicked(false);
    if (info)
        load(info);
    resize(180,120);
}

SendLaterDialog::~SendLaterDialog()
{
    delete mSendLaterWidget;
}

void SendLaterDialog::slotRecurrenceClicked(bool clicked)
{
    mSendLaterWidget->mRecurrenceValue->setEnabled(clicked);
    mSendLaterWidget->mRecurrenceComboBox->setEnabled(clicked);
}

void SendLaterDialog::load(SendLater::SendLaterInfo *info)
{
    mSendLaterWidget->mDateTime->setDateTime(info->dateTime());
    const bool recurrence = info->isRecurrence();
    mSendLaterWidget->mRecurrence->setChecked(recurrence);
    slotRecurrenceClicked(recurrence);
    mSendLaterWidget->mRecurrenceValue->setValue(info->recurrenceEachValue());
    mSendLaterWidget->mRecurrenceComboBox->setCurrentIndex((int)info->recurrenceUnit());
}

SendLater::SendLaterInfo* SendLaterDialog::info()
{
    if (!mInfo) {
        mInfo = new SendLater::SendLaterInfo();
    }
    mInfo->setRecurrence(mSendLaterWidget->mRecurrence->isChecked());
    mInfo->setRecurrenceEachValue(mSendLaterWidget->mRecurrenceValue->value());
    mInfo->setRecurrenceUnit((SendLater::SendLaterInfo::RecurrenceUnit)mSendLaterWidget->mRecurrenceComboBox->currentIndex());
    if (mSendDateTime.isValid())
        mInfo->setDateTime(mSendDateTime);
    else
        mInfo->setDateTime(mSendLaterWidget->mDateTime->dateTime());
    return mInfo;
}

SendLaterDialog::SendLaterAction SendLaterDialog::action() const
{
    return mAction;
}

void SendLaterDialog::slotOkClicked()
{
    if (mDelay->isChecked()) {
        mSendDateTime = mSendLaterWidget->mDateTime->dateTime();
        mAction = SendDeliveryAtTime;
    } else {
        mAction = PutInOutbox;
    }
    accept();
}

void SendLaterDialog::slotDelay(bool delayEnabled)
{
    mSendLaterWidget->mDateTime->setEnabled(delayEnabled);
    mSendLaterWidget->mRecurrence->setEnabled(delayEnabled);
    mSendLaterWidget->mRecurrenceValue->setEnabled(delayEnabled && mSendLaterWidget->mRecurrence->isChecked());
    mSendLaterWidget->mRecurrenceComboBox->setEnabled(delayEnabled && mSendLaterWidget->mRecurrence->isChecked());
}

