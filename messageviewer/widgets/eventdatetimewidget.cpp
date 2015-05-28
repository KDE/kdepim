/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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
    const KDateTime currentDateTime = KDateTime::currentDateTime(KDateTime::LocalZone);
    setDateTime(currentDateTime);
    connect(mDateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(slotDateTimeChanged()));
    connect(mTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(slotDateTimeChanged()));
    connect(mDateEdit, SIGNAL(dateEdited(QDate)), this, SLOT(slotDateTimeChanged()));
    connect(mTimeEdit, SIGNAL(timeEdited(QTime)), this, SLOT(slotDateTimeChanged()));
}

EventDateTimeWidget::~EventDateTimeWidget()
{

}

void EventDateTimeWidget::slotDateTimeChanged()
{
    Q_EMIT dateTimeChanged(dateTime());
}

void EventDateTimeWidget::setMinimumDateTime(const KDateTime &dtime)
{
    if (dateTime() != dtime) {
        mDateEdit->setMinimumDate(dtime.date());
        mTimeEdit->setMinimumTime(dtime.time());
    }
}

void EventDateTimeWidget::setDateTime(const KDateTime &dTime)
{
    if (dateTime() != dTime) {
        blockSignals(true);
        mDateEdit->setDate(dTime.date());
        mTimeEdit->setTime(dTime.time());
        blockSignals(false);
        slotDateTimeChanged();
    }
}

KDateTime EventDateTimeWidget::dateTime() const
{
    KDateTime dateTime = KDateTime::currentLocalDateTime();
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
