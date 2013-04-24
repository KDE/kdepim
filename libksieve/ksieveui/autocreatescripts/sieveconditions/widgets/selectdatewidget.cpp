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

#include "selectdatewidget.h"

#include <KLocale>
#include <KComboBox>
#include <KLineEdit>

#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>

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

    mDateType = new KComboBox;
    connect(mDateType, SIGNAL(activated(int)), SLOT(slotDateTypeActivated(int)));
    mDateType->addItem(i18n("Year"), QLatin1String("year"));
    mDateType->addItem(i18n("Month"), QLatin1String("month"));
    mDateType->addItem(i18n("Day"), QLatin1String("day"));
    mDateType->addItem(i18n("Date"), QLatin1String("date"));
    mDateType->addItem(i18n("Julian"), QLatin1String("julian"));
    mDateType->addItem(i18n("Hour"), QLatin1String("hour"));
    mDateType->addItem(i18n("Minute"), QLatin1String("minute"));
    mDateType->addItem(i18n("Second"), QLatin1String("second"));
    mDateType->addItem(i18n("Time"), QLatin1String("time"));
    mDateType->addItem(i18n("iso8601"), QLatin1String("iso8601"));
    mDateType->addItem(i18n("std11"), QLatin1String("std11"));
    mDateType->addItem(i18n("Zone"), QLatin1String("zone"));
    mDateType->addItem(i18n("Weekday"), QLatin1String("weekday"));
    lay->addWidget(mDateType);

    QLabel *lab = new QLabel(i18n("value:"));
    lay->addWidget(lab);

    mStackWidget = new QStackedWidget;
    lay->addWidget(mStackWidget);

    mDateEdit = new KLineEdit;
    mStackWidget->addWidget(mDateEdit);



    setLayout(lay);
}

void SelectDateWidget::slotDateTypeActivated(int index)
{
    const QString itemData = mDateType->itemData(index).toString();
    if (itemData == QLatin1String("year")) {
        //TODO
    }
}

QString SelectDateWidget::code() const
{
    return QString::fromLatin1("\"%1\" \"%2\"").arg(mDateType->itemData(mDateType->currentIndex()).toString()).arg(mDateEdit->text());
}

#include "selectdatewidget.moc"
