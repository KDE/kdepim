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

#include "sieveconditiondate.h"
#include "widgets/selectmatchtypecombobox.h"
#include "widgets/selectdatewidget.h"

#include <KLocale>

#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

using namespace KSieveUi;

SieveConditionDate::SieveConditionDate(QObject *parent)
    : SieveCondition(QLatin1String("date"), i18n("Date"), parent)
{
}

SieveCondition *SieveConditionDate::newAction()
{
    return new SieveConditionDate;
}

QWidget *SieveConditionDate::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectMatchTypeComboBox *matchTypeCombo = new SelectMatchTypeComboBox;
    matchTypeCombo->setObjectName(QLatin1String("matchtype"));
    lay->addWidget(matchTypeCombo);

    QLabel *lab = new QLabel(i18n("header"));
    lay->addWidget(lab);

    SelectDateWidget *dateWidget = new SelectDateWidget;
    dateWidget->setObjectName(QLatin1String("datewidget"));
    lay->addWidget(dateWidget);

    return w;
}

QString SieveConditionDate::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *selectMatchCombobox = w->findChild<SelectMatchTypeComboBox*>(QLatin1String("matchtype"));
    bool isNegative = false;
    const QString matchTypeStr = selectMatchCombobox->code(isNegative);

    const SelectDateWidget *dateWidget = w->findChild<SelectDateWidget*>(QLatin1String("datewidget"));
    const QString dateWidgetStr = dateWidget->code();

    return (isNegative ? QLatin1String("not ") : QString()) + QString::fromLatin1("date %1 %2").arg(matchTypeStr).arg(dateWidgetStr);
}

bool SieveConditionDate::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionDate::serverNeedsCapability() const
{
    return QLatin1String("date");
}

#include "sieveconditiondate.moc"

