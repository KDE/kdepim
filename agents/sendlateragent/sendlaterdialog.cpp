/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include <KLocalizedString>
#include <KSeparator>
#include <QIcon>

#include <QVBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace SendLater;

SendLaterDialog::SendLaterDialog(SendLater::SendLaterInfo *info, QWidget *parent)
    : QDialog(parent),
      mAction(Unknown),
      mDelay(0),
      mInfo(info)
{
    setWindowTitle( i18nc("@title:window", "Send Later") );
    setWindowIcon( QIcon::fromTheme( QLatin1String("kmail") ) );

    QWidget *sendLaterWidget = new QWidget;
    mSendLaterWidget = new Ui::SendLaterWidget;
    mSendLaterWidget->setupUi( sendLaterWidget );

    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    w->setLayout(lay);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SendLaterDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SendLaterDialog::reject);

    if (!info) {
        okButton->setText(i18n("Send Later"));
        mDelay = new QCheckBox(i18n("Delay"));
        mDelay->setChecked(false);
        slotDelay(false);
        connect(mDelay, &QCheckBox::clicked, this, &SendLaterDialog::slotDelay);
        lay->addWidget(mDelay);
    }

    connect(okButton, &QPushButton::clicked, this, &SendLaterDialog::slotOkClicked);

    lay->addWidget(sendLaterWidget);

    QDateTime t = QDateTime::currentDateTime();
    t = t.addSecs(60*60);

    mSendLaterWidget->mDateTime->setDateTime(t);
    connect(mSendLaterWidget->mRecurrence, &QCheckBox::clicked, this, &SendLaterDialog::slotRecurrenceClicked);
    QStringList unitsList;
    unitsList<<i18n("Days");
    unitsList<<i18n("Weeks");
    unitsList<<i18n("Months");
    unitsList<<i18n("Years");
    mSendLaterWidget->mRecurrenceComboBox->addItems(unitsList);

    lay->addWidget(new KSeparator);

    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);
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
    if (!mDelay || (mDelay && mDelay->isChecked())) {
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

