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
      mAction(Unknown),
      mInfo(info)
{
    setCaption( i18n("Send Later") );
    setWindowIcon( KIcon( QLatin1String("kmail") ) );
    setButtons( Ok|Cancel );
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));

    if (!info) {
        setButtonText( Ok, i18n("Send Later"));
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
    hbox->addWidget(lab);
    hbox->addWidget(mDateTime);

    hbox = new QHBoxLayout;
    lay->addLayout(hbox);

    mRecurrence = new QCheckBox(i18n("Recurrence"));
    connect(mRecurrence, SIGNAL(clicked(bool)), this, SLOT(slotRecurrenceClicked(bool)));
    hbox->addWidget(mRecurrence);


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

    lay->addWidget(new KSeparator);

    w->setLayout(lay);
    setMainWidget(w);
    slotRecurrenceClicked(false);
    if (info)
        load(info);
    resize( 400,300);
}

SendLaterDialog::~SendLaterDialog()
{
}

void SendLaterDialog::slotRecurrenceClicked(bool clicked)
{
    mRecurrenceValue->setEnabled(clicked);
    mRecurrenceComboBox->setEnabled(clicked);
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

SendLaterDialog::SendLaterAction SendLaterDialog::action() const
{
    return mAction;
}

void SendLaterDialog::slotOkClicked()
{
    mSendDateTime = mDateTime->dateTime();
    mAction = SendDeliveryAtTime;
    accept();
}

#include "sendlaterdialog.moc"
