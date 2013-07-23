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

#include "sieveconditioncurrentdate.h"
#include "autocreatescripts/commonwidgets/selectmatchtypecombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "widgets/selectdatewidget.h"

#include <KLocale>

#include <QHBoxLayout>

using namespace KSieveUi;

SieveConditionCurrentDate::SieveConditionCurrentDate(QObject *parent)
    : SieveCondition(QLatin1String("currentdate"), i18n("Currentdate"), parent)
{
}

SieveCondition *SieveConditionCurrentDate::newAction()
{
    return new SieveConditionCurrentDate;
}

QWidget *SieveConditionCurrentDate::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectMatchTypeComboBox *matchTypeCombo = new SelectMatchTypeComboBox;
    matchTypeCombo->setObjectName(QLatin1String("matchtype"));
    lay->addWidget(matchTypeCombo);

    SelectDateWidget *dateWidget = new SelectDateWidget;
    dateWidget->setObjectName(QLatin1String("datewidget"));
    lay->addWidget(dateWidget);

    return w;
}

QString SieveConditionCurrentDate::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *selectMatchCombobox = w->findChild<SelectMatchTypeComboBox*>(QLatin1String("matchtype"));
    bool isNegative = false;
    const QString matchTypeStr = selectMatchCombobox->code(isNegative);

    const SelectDateWidget *dateWidget = w->findChild<SelectDateWidget*>(QLatin1String("datewidget"));
    const QString dateWidgetStr = dateWidget->code();

    return AutoCreateScriptUtil::negativeString(isNegative) + QString::fromLatin1("currentdate %1 %2").arg(matchTypeStr).arg(dateWidgetStr);
}

bool SieveConditionCurrentDate::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionCurrentDate::serverNeedsCapability() const
{
    return QLatin1String("date");
}

QStringList SieveConditionCurrentDate::needRequires(QWidget *) const
{
    return QStringList() << QLatin1String("date");
}

QString SieveConditionCurrentDate::help() const
{
    return i18n("The currentdate test is similar to the date test, except that it operates on the current date/time rather than a value extracted from the message header.");
}

void SieveConditionCurrentDate::setParamWidgetValue(const QDomElement &element, QWidget *parent, bool notCondition )
{

}

#include "sieveconditioncurrentdate.moc"
