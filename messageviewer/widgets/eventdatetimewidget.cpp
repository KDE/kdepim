/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "eventdatetimewidget.h"
#include <KDateComboBox>
#include <KTimeComboBox>
#include <QHBoxLayout>
#include <QDebug>
#include <QDateTime>

using namespace MessageViewer;
EventDateTimeWidget::EventDateTimeWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    setLayout(mainLayout);
    mDateEdit = new KDateComboBox;
    mDateEdit->setObjectName(QLatin1String("eventdatecombobox"));
    mainLayout->addWidget(mDateEdit);
    mTimeEdit = new KTimeComboBox;
    mTimeEdit->setObjectName(QLatin1String("eventtimecombobox"));
    mainLayout->addWidget(mTimeEdit);
    const QDateTime currentDateTime = QDateTime::currentDateTime();
    setDateTime(currentDateTime);
    connect(mDateEdit, &KDateComboBox::dateChanged, this, &EventDateTimeWidget::slotDateTimeChanged);
    connect(mTimeEdit, &KTimeComboBox::timeChanged, this, &EventDateTimeWidget::slotDateTimeChanged);
    connect(mDateEdit, &KDateComboBox::dateEdited, this, &EventDateTimeWidget::slotDateTimeChanged);
    connect(mTimeEdit, &KTimeComboBox::timeEdited, this, &EventDateTimeWidget::slotDateTimeChanged);
}

EventDateTimeWidget::~EventDateTimeWidget()
{

}

void EventDateTimeWidget::slotDateTimeChanged()
{
    Q_EMIT dateTimeChanged(dateTime());
}

void EventDateTimeWidget::setMinimumDateTime(const QDateTime &dtime)
{
    if (dateTime() != dtime) {
        mDateEdit->setMinimumDate(dtime.date());
        mTimeEdit->setMaximumTime(dtime.time());
    }
}

void EventDateTimeWidget::setDateTime(const QDateTime &dTime)
{
    if (dateTime() != dTime) {
        blockSignals(true);
        mDateEdit->setDate(dTime.date());
        mTimeEdit->setTime(dTime.time());
        blockSignals(false);
        slotDateTimeChanged();
    }
}

QDateTime EventDateTimeWidget::dateTime() const
{
    QDateTime dateTime = QDateTime::currentDateTime();
    dateTime.setTime(mTimeEdit->time());
    dateTime.setDate(mDateEdit->date());
    return dateTime;
}

QDate EventDateTimeWidget::date() const
{
    return mDateEdit->date();
}

QTime EventDateTimeWidget::time() const
{
    return mTimeEdit->time();
}

void EventDateTimeWidget::setTime(const QTime &time)
{
    mTimeEdit->setTime(time);
}

void EventDateTimeWidget::setDate(const QDate &date)
{
    mDateEdit->setDate(date);
}
