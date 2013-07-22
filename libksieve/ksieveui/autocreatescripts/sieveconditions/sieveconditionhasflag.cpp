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

#include "sieveconditionhasflag.h"
#include "autocreatescripts/commonwidgets/selectmatchtypecombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include <KLocale>
#include <KLineEdit>

#include <QWidget>
#include <QHBoxLayout>
#include <QDebug>
#include <QLabel>

using namespace KSieveUi;
SieveConditionHasFlag::SieveConditionHasFlag(QObject *parent)
    : SieveCondition(QLatin1String("hasflag"), i18n("Has Flag"), parent)
{
}

SieveCondition *SieveConditionHasFlag::newAction()
{
    return new SieveConditionHasFlag;
}

QWidget *SieveConditionHasFlag::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);
    SelectMatchTypeComboBox *selecttype = new SelectMatchTypeComboBox;
    selecttype->setObjectName(QLatin1String("matchtype"));
    lay->addWidget(selecttype);

    QLabel *lab = new QLabel(i18n("Variable name (if empty it uses internal variable):"));
    lay->addWidget(lab);

    KLineEdit *variableName = new KLineEdit;
    variableName->setObjectName(QLatin1String("variablename"));
    lay->addWidget(variableName);

    lab = new QLabel(i18n("Value:"));
    lay->addWidget(lab);

    KLineEdit *value = new KLineEdit;
    value->setObjectName(QLatin1String("value"));
    lay->addWidget(value);

    return w;
}

QString SieveConditionHasFlag::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *matchTypeCombo = w->findChild<SelectMatchTypeComboBox*>( QLatin1String("matchtype") );
    bool isNegative = false;
    const QString matchString = matchTypeCombo->code(isNegative);

    QString result = AutoCreateScriptUtil::negativeString(isNegative) + QString::fromLatin1("hasflag %1").arg(matchString);

    const KLineEdit *variableName = w->findChild<KLineEdit*>(QLatin1String("variablename"));
    const QString variableNameStr = variableName->text();
    if (!variableNameStr.isEmpty()) {
        result += variableNameStr + QLatin1Char(' ');
    }

    const KLineEdit *value = w->findChild<KLineEdit*>(QLatin1String("value"));
    const QString valueStr = value->text();
    result += valueStr;
    return result;
}

QStringList SieveConditionHasFlag::needRequires(QWidget *) const
{
    return QStringList() << QLatin1String("imap4flags");
}

bool SieveConditionHasFlag::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionHasFlag::serverNeedsCapability() const
{
    return QLatin1String("imap4flags");
}

QString SieveConditionHasFlag::help() const
{
    return i18n("The hasflag test evaluates to true if any of the variables matches any flag name.");
}

void SieveConditionHasFlag::setParamWidgetValue(const QDomElement &element, QWidget *parent, bool notCondition )
{

}

#include "sieveconditionhasflag.moc"

