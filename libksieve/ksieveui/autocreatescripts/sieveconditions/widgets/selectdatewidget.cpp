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

#include "selectdatewidget.h"

#include <KLocalizedString>
#include <KComboBox>
#include <KLineEdit>
#include <KDateComboBox>
#include <KTimeComboBox>

#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QSpinBox>
#include <QDebug>

Q_DECLARE_METATYPE(KSieveUi::SelectDateWidget::DateType)

using namespace KSieveUi;
SelectDateWidget::SelectDateWidget(QWidget *parent)
    : QWidget(parent)
{
    initialize();
}

SelectDateWidget::~SelectDateWidget()
{

}

void SelectDateWidget::initialize()
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);

    mDateType = new KComboBox;
    connect(mDateType, SIGNAL(activated(int)), SLOT(slotDateTypeActivated(int)));
    mDateType->addItem(i18n("Year"), QVariant::fromValue(KSieveUi::SelectDateWidget::Year));
    mDateType->addItem(i18n("Month"), QVariant::fromValue(KSieveUi::SelectDateWidget::Month));
    mDateType->addItem(i18n("Day"), QVariant::fromValue(KSieveUi::SelectDateWidget::Day));
    mDateType->addItem(i18n("Date"), QVariant::fromValue(KSieveUi::SelectDateWidget::Date));
    mDateType->addItem(i18n("Julian"), QVariant::fromValue(KSieveUi::SelectDateWidget::Julian));
    mDateType->addItem(i18n("Hour"), QVariant::fromValue(KSieveUi::SelectDateWidget::Hour));
    mDateType->addItem(i18n("Minute"), QVariant::fromValue(KSieveUi::SelectDateWidget::Minute));
    mDateType->addItem(i18n("Second"), QVariant::fromValue(KSieveUi::SelectDateWidget::Second));
    mDateType->addItem(i18n("Time"), QVariant::fromValue(KSieveUi::SelectDateWidget::Time));
    mDateType->addItem(i18n("iso8601"), QVariant::fromValue(KSieveUi::SelectDateWidget::Iso8601));
    mDateType->addItem(i18n("std11"), QVariant::fromValue(KSieveUi::SelectDateWidget::Std11));
    mDateType->addItem(i18n("Zone"), QVariant::fromValue(KSieveUi::SelectDateWidget::Zone));
    mDateType->addItem(i18n("Weekday"), QVariant::fromValue(KSieveUi::SelectDateWidget::Weekday));
    lay->addWidget(mDateType);

    QLabel *lab = new QLabel(i18n("value:"));
    lay->addWidget(lab);

    mStackWidget = new QStackedWidget;
    lay->addWidget(mStackWidget);

    mDateLineEdit = new KLineEdit;
    mStackWidget->addWidget(mDateLineEdit);

    mDateValue = new QSpinBox;
    mStackWidget->addWidget(mDateValue);

    mDateEdit = new KDateComboBox;
    mStackWidget->addWidget(mDateEdit);

    mTimeEdit = new KTimeComboBox;
    mStackWidget->addWidget(mTimeEdit);

    mStackWidget->setCurrentWidget(mDateValue);

    setLayout(lay);
}

void SelectDateWidget::slotDateTypeActivated(int index)
{
    const DateType type = mDateType->itemData(index).value<KSieveUi::SelectDateWidget::DateType>();
    switch(type) {
    case Year:
        mStackWidget->setCurrentWidget(mDateValue);
        mDateValue->setMinimum(0);
        mDateValue->setMaximum(9999);
        break;
    case Month:
        mStackWidget->setCurrentWidget(mDateValue);
        mDateValue->setMinimum(1);
        mDateValue->setMaximum(12);
        break;
    case Day:
        mStackWidget->setCurrentWidget(mDateValue);
        mDateValue->setMinimum(1);
        mDateValue->setMaximum(31);
        break;
    case Hour:
        mStackWidget->setCurrentWidget(mDateValue);
        mDateValue->setMinimum(0);
        mDateValue->setMaximum(23);
        break;
    case Minute:
        mDateValue->setMinimum(0);
        mDateValue->setMaximum(59);
        mStackWidget->setCurrentWidget(mDateValue);
        break;
    case Second:
        mDateValue->setMinimum(0);
        mDateValue->setMaximum(60);
        mStackWidget->setCurrentWidget(mDateValue);
        break;
    case Weekday:
        mDateValue->setMinimum(0);
        mDateValue->setMaximum(6);
        mStackWidget->setCurrentWidget(mDateValue);
        break;
    case Date:
        mStackWidget->setCurrentWidget(mDateEdit);
        break;
    case Julian:
        mStackWidget->setCurrentWidget(mDateLineEdit);
        break;
    case Time:
        mStackWidget->setCurrentWidget(mTimeEdit);
        break;
    case Iso8601:
        mStackWidget->setCurrentWidget(mDateLineEdit);
        break;
    case Std11:
        mStackWidget->setCurrentWidget(mDateLineEdit);
        break;
    case Zone:
        mStackWidget->setCurrentWidget(mDateLineEdit);
        break;
    }
}

QString SelectDateWidget::dateValue(SelectDateWidget::DateType type) const
{
    QString str;
    switch(type) {
    case Year:
        str = QString::fromLatin1("%1").arg(mDateValue->value(),4, 10, QLatin1Char('0'));
        break;
    case Month:
        str = QString::fromLatin1("%1").arg(mDateValue->value(),2, 10,QLatin1Char('0'));
        break;
    case Day:
        str = QString::fromLatin1("%1").arg(mDateValue->value(),2, 10, QLatin1Char('0'));
        break;
    case Date:
        str = mDateEdit->date().toString();
        break;
    case Julian:
        //TODO
        str = mDateEdit->date().toString();
        break;
    case Hour:
        str = QString::fromLatin1("%1").arg(mDateValue->value(),2, 10, QLatin1Char('0'));
        break;
    case Minute:
        str = QString::fromLatin1("%1").arg(mDateValue->value(),2, 10, QLatin1Char('0'));
        break;
    case Second:
        str = QString::fromLatin1("%1").arg(mDateValue->value(),2, 10, QLatin1Char('0'));
        break;
    case Time:
        str = mTimeEdit->time().toString();
        break;
    case Iso8601:
        str = mDateEdit->date().toString();
        break;
    case Std11:
        str = mDateEdit->date().toString();
        break;
    case Zone:
        str = mDateEdit->date().toString();
        break;
    case Weekday:
        str = QString::fromLatin1("%1").arg(mDateValue->value());
        break;
    }
    return str;
}

SelectDateWidget::DateType SelectDateWidget::dateTypeFromString(const QString &str)
{
    if (str == QLatin1String("year")) {
        return Year;
    } else if (str == QLatin1String("month")) {
        return Month;
    } else if (str == QLatin1String("day")) {
        return Day;
    } else if (str == QLatin1String("date")) {
        return Date;
    } else if (str == QLatin1String("julian")) {
        return Julian;
    } else if (str == QLatin1String("hour")) {
        return Hour;
    } else if (str == QLatin1String("minute")) {
        return Minute;
    } else if (str == QLatin1String("second")) {
        return Second;
    } else if (str == QLatin1String("time")) {
        return Time;
    } else if (str == QLatin1String("iso8601")) {
        return Iso8601;
    } else if (str == QLatin1String("std11")) {
        return Std11;
    } else if (str == QLatin1String("zone")) {
        return Zone;
    } else if (str == QLatin1String("weekday")) {
        return Weekday;
    } else {
        qDebug()<<" date type unknown :"<<str;
    }
    return Year;
}

QString SelectDateWidget::dateType(SelectDateWidget::DateType type) const
{
    QString str;
    switch(type) {
    case Year:
        str = QLatin1String("year");
        break;
    case Month:
        str = QLatin1String("month");
        break;
    case Day:
        str = QLatin1String("day");
        break;
    case Date:
        str = QLatin1String("date");
        break;
    case Julian:
        str = QLatin1String("julian");
        break;
    case Hour:
        str = QLatin1String("hour");
        break;
    case Minute:
        str = QLatin1String("minute");
        break;
    case Second:
        str = QLatin1String("second");
        break;
    case Time:
        str = QLatin1String("time");
        break;
    case Iso8601:
        str = QLatin1String("iso8601");
        break;
    case Std11:
        str = QLatin1String("std11");
        break;
    case Zone:
        str = QLatin1String("zone");
        break;
    case Weekday:
        str = QLatin1String("weekday");
        break;
    }
    return str;
}

QString SelectDateWidget::code() const
{
    const DateType type = mDateType->itemData(mDateType->currentIndex()).value<KSieveUi::SelectDateWidget::DateType>();
    return QString::fromLatin1("\"%1\" \"%2\"").arg(dateType(type)).arg(dateValue(type));
}

void SelectDateWidget::setCode(const QString &type, const QString &value)
{
    const int index = dateTypeFromString(type);
    if (index != -1) {
        mDateType->setCurrentIndex(index);
    } else {
        mDateType->setCurrentIndex(0);
    }
    const DateType dateType = mDateType->itemData(index).value<KSieveUi::SelectDateWidget::DateType>();
    switch(dateType) {
    case Month:
    case Day:
    case Hour:
    case Minute:
    case Second:
    case Weekday:
    case Year:
        mStackWidget->setCurrentWidget(mDateValue);
        mDateValue->setValue(value.toInt());
        break;
    case Date:
        mStackWidget->setCurrentWidget(mDateEdit);
        mDateEdit->setDate(QDate::fromString(value));
        break;
    case Julian:
        mStackWidget->setCurrentWidget(mDateLineEdit);
        mDateLineEdit->setText(value);
        break;
    case Time:
        mStackWidget->setCurrentWidget(mTimeEdit);
        mTimeEdit->setTime(QTime::fromString(value));
        break;
    case Iso8601:
    case Std11:
    case Zone:
        mStackWidget->setCurrentWidget(mDateLineEdit);
        mDateLineEdit->setText(value);
        break;
    }

}

