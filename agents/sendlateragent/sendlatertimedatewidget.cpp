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

#include "sendlatertimedatewidget.h"

#include <KTimeComboBox>
#include <KDateComboBox>

#include <QHBoxLayout>

using namespace SendLater;

SendLaterTimeDateWidget::SendLaterTimeDateWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);

    QDateTime t = QDateTime::currentDateTime();
    t = t.addSecs(60*60);
    mTimeComboBox = new KTimeComboBox;
    connect(mTimeComboBox, SIGNAL(timeChanged(QTime)), this, SLOT(slotDateTimeChanged()));

    mDateComboBox = new KDateComboBox;
    mDateComboBox->setOptions(KDateComboBox::EditDate|KDateComboBox::SelectDate|KDateComboBox::DatePicker|KDateComboBox::DateKeywords|KDateComboBox::WarnOnInvalid);
    mDateComboBox->setMinimumDate(t.date(), i18n("You cannot select a date prior to the current date."));
    connect(mDateComboBox, SIGNAL(dateChanged(QDate)), this, SLOT(slotDateTimeChanged()));

    lay->addWidget(mDateComboBox);
    lay->addWidget(mTimeComboBox);

    setLayout(lay);
}

SendLaterTimeDateWidget::~SendLaterTimeDateWidget()
{

}

void SendLaterTimeDateWidget::slotDateTimeChanged()
{
    QDateTime dt;
    dt.setDate(mDateComboBox->date());
    dt.setTime(mTimeComboBox->time());
    Q_EMIT dateTimeChanged(dt);
}

QDateTime SendLaterTimeDateWidget::dateTime() const
{
    QDateTime dt;
    dt.setDate(mDateComboBox->date());
    dt.setTime(mTimeComboBox->time());
    return dt;
}

void SendLaterTimeDateWidget::setDateTime(const QDateTime &datetime)
{
    mTimeComboBox->setTime(datetime.time());
    mDateComboBox->setDate(datetime.date());
}

