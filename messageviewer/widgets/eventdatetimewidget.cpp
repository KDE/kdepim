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

#include <QDateTime>

using namespace MessageViewer;
class MessageViewer::EventDateTimeWidgetPrivate
{
public:
    EventDateTimeWidgetPrivate()
        : mDateEdit(Q_NULLPTR),
          mTimeEdit(Q_NULLPTR)
    {

    }
    KDateComboBox *mDateEdit;
    KTimeComboBox *mTimeEdit;
};

EventDateTimeWidget::EventDateTimeWidget(QWidget *parent)
    : QWidget(parent),
      d(new MessageViewer::EventDateTimeWidgetPrivate)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    setLayout(mainLayout);
    d->mDateEdit = new KDateComboBox;
    d->mDateEdit->setObjectName(QStringLiteral("eventdatecombobox"));
    mainLayout->addWidget(d->mDateEdit);
    d->mTimeEdit = new KTimeComboBox;
    d->mTimeEdit->setObjectName(QStringLiteral("eventtimecombobox"));
    mainLayout->addWidget(d->mTimeEdit);
    const QDateTime currentDateTime = QDateTime::currentDateTime();
    setDateTime(currentDateTime);
    connect(d->mDateEdit, &KDateComboBox::dateChanged, this, &EventDateTimeWidget::slotDateTimeChanged);
    connect(d->mTimeEdit, &KTimeComboBox::timeChanged, this, &EventDateTimeWidget::slotDateTimeChanged);
    connect(d->mDateEdit, &KDateComboBox::dateEdited, this, &EventDateTimeWidget::slotDateTimeChanged);
    connect(d->mTimeEdit, &KTimeComboBox::timeEdited, this, &EventDateTimeWidget::slotDateTimeChanged);
}

EventDateTimeWidget::~EventDateTimeWidget()
{
    delete d;
}

void EventDateTimeWidget::slotDateTimeChanged()
{
    Q_EMIT dateTimeChanged(dateTime());
}

void EventDateTimeWidget::setMinimumDateTime(const QDateTime &dtime)
{
    if (dateTime() != dtime) {
        d->mDateEdit->setMinimumDate(dtime.date());
        d->mTimeEdit->setMinimumTime(dtime.time());
    }
}

void EventDateTimeWidget::setDateTime(const QDateTime &dTime)
{
    if (dateTime() != dTime) {
        blockSignals(true);
        d->mDateEdit->setDate(dTime.date());
        d->mTimeEdit->setTime(dTime.time());
        blockSignals(false);
        slotDateTimeChanged();
    }
}

QDateTime EventDateTimeWidget::dateTime() const
{
    QDateTime dateTime = QDateTime::currentDateTime();
    dateTime.setTime(d->mTimeEdit->time());
    dateTime.setDate(d->mDateEdit->date());
    return dateTime;
}

QDate EventDateTimeWidget::date() const
{
    return d->mDateEdit->date();
}

QTime EventDateTimeWidget::time() const
{
    return d->mTimeEdit->time();
}

void EventDateTimeWidget::setTime(const QTime &time)
{
    d->mTimeEdit->setTime(time);
}

void EventDateTimeWidget::setDate(const QDate &date)
{
    d->mDateEdit->setDate(date);
}
