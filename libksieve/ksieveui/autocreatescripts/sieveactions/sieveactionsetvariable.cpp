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

#include "sieveactionsetvariable.h"
#include "widgets/selectvariablemodifiercombobox.h"

#include <KLocale>
#include <KLineEdit>

#include <QHBoxLayout>
#include <QLabel>

using namespace KSieveUi;
SieveActionSetVariable::SieveActionSetVariable(QObject *parent)
    : SieveAction(QLatin1String("variable"), i18n("Variable"), parent)
{
}

SieveAction* SieveActionSetVariable::newAction()
{
    return new SieveActionSetVariable;
}

QWidget *SieveActionSetVariable::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);


    SelectVariableModifierComboBox *modifier = new SelectVariableModifierComboBox;
    modifier->setObjectName(QLatin1String("modifier"));
    lay->addWidget(modifier);

    QLabel *lab = new QLabel(i18n("Value:"));
    lay->addWidget(lab);

    KLineEdit *value = new KLineEdit;
    value->setObjectName(QLatin1String("value"));
    lay->addWidget(value);

    lab = new QLabel(i18n("In variable:"));
    lay->addWidget(lab);

    KLineEdit *variable = new KLineEdit;
    variable->setObjectName(QLatin1String("variable"));
    lay->addWidget(variable);

    return w;
}

void SieveActionSetVariable::setParamWidgetValue(const QDomElement &element, QWidget *w )
{
    SelectVariableModifierComboBox *modifier = w->findChild<SelectVariableModifierComboBox*>(QLatin1String("modifier"));
    KLineEdit *value = w->findChild<KLineEdit*>(QLatin1String("value"));
    KLineEdit *variable = w->findChild<KLineEdit*>(QLatin1String("variable"));
    //TODO
}

QString SieveActionSetVariable::code(QWidget *w) const
{
    QString result = QLatin1String("set ");
    const SelectVariableModifierComboBox *modifier = w->findChild<SelectVariableModifierComboBox*>(QLatin1String("modifier"));
    const QString modifierStr = modifier->code();
    if (!modifierStr.isEmpty()) {
        result += modifierStr + QLatin1Char(' ');
    }
    const KLineEdit *value = w->findChild<KLineEdit*>(QLatin1String("value"));
    const QString valueStr = value->text();
    result += QString::fromLatin1("\"%1\" ").arg(valueStr);

    const KLineEdit *variable = w->findChild<KLineEdit*>(QLatin1String("variable"));
    const QString variableStr = variable->text();
    result += QString::fromLatin1("\"%1\";").arg(variableStr);


    return result;
}

QStringList SieveActionSetVariable::needRequires(QWidget *) const
{
    return QStringList() << QLatin1String("variables");
}

bool SieveActionSetVariable::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionSetVariable::serverNeedsCapability() const
{
    return QLatin1String("variables");
}

QString SieveActionSetVariable::help() const
{
    return i18n("The \"set\" action stores the specified value in the variable identified by name.");
}

#include "sieveactionsetvariable.moc"
