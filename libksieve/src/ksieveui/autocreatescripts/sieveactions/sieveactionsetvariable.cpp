/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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
#include "editor/sieveeditorutil.h"
#include "widgets/selectvariablemodifiercombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "autocreatescripts/sieveeditorgraphicalmodewidget.h"

#include <KLocalizedString>
#include <QLineEdit>

#include <QCheckBox>
#include <QLabel>
#include <QDomNode>
#include "libksieve_debug.h"
#include <QGridLayout>

using namespace KSieveUi;
SieveActionSetVariable::SieveActionSetVariable(QObject *parent)
    : SieveAction(QStringLiteral("set"), i18n("Variable"), parent)
{
    mHasRegexCapability = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QStringLiteral("regex"));
}

SieveAction *SieveActionSetVariable::newAction()
{
    return new SieveActionSetVariable;
}

QWidget *SieveActionSetVariable::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    w->setLayout(grid);

    SelectVariableModifierComboBox *modifier = new SelectVariableModifierComboBox;
    modifier->setObjectName(QStringLiteral("modifier"));
    connect(modifier, &SelectVariableModifierComboBox::valueChanged, this, &SieveActionSetVariable::valueChanged);
    grid->addWidget(modifier, 0, 0);

    if (mHasRegexCapability) {
        QCheckBox *protectAgainstUseRegexp = new QCheckBox(i18n("Protect special character"));
        connect(protectAgainstUseRegexp, &QCheckBox::clicked, this, &SieveActionSetVariable::valueChanged);
        protectAgainstUseRegexp->setObjectName(QStringLiteral("regexprotect"));
        grid->addWidget(protectAgainstUseRegexp, 0, 1);
    }

    QLabel *lab = new QLabel(i18n("Value:"));
    grid->addWidget(lab, 1, 0);

    QLineEdit *value = new QLineEdit;
    value->setObjectName(QStringLiteral("value"));
    connect(value, &QLineEdit::textChanged, this, &SieveActionSetVariable::valueChanged);
    grid->addWidget(value, 1, 1);

    lab = new QLabel(i18n("In variable:"));
    grid->addWidget(lab, 2, 0);

    QLineEdit *variable = new QLineEdit;
    variable->setObjectName(QStringLiteral("variable"));
    connect(variable, &QLineEdit::textChanged, this, &SieveActionSetVariable::valueChanged);
    grid->addWidget(variable, 2, 1);

    return w;
}

QUrl SieveActionSetVariable::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

bool SieveActionSetVariable::setParamWidgetValue(const QDomElement &element, QWidget *w, QString &error)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                const QString tagValue = e.text();
                QLineEdit *value = w->findChild<QLineEdit *>(QStringLiteral("value"));
                value->setText(tagValue);
                node = node.nextSibling();
                QDomElement variableElement = node.toElement();
                if (!variableElement.isNull()) {
                    const QString variableTagName = variableElement.tagName();
                    if (variableTagName == QLatin1String("str")) {
                        QLineEdit *variable = w->findChild<QLineEdit *>(QStringLiteral("variable"));
                        variable->setText(variableElement.text());
                    }
                } else {
                    return false;
                }
            } else if (tagName == QLatin1String("tag")) {
                const QString tagValue = e.text();
                if (tagValue == QLatin1String("quoteregex")) {
                    if (mHasRegexCapability) {
                        QCheckBox *protectAgainstUseRegexp = w->findChild<QCheckBox *>(QStringLiteral("regexprotect"));
                        protectAgainstUseRegexp->setChecked(true);
                    } else {
                        error += QLatin1Char('\n') + i18n("Script needs regex support, but server does not have it.");
                    }
                } else {
                    SelectVariableModifierComboBox *modifier = w->findChild<SelectVariableModifierComboBox *>(QStringLiteral("modifier"));
                    modifier->setCode(AutoCreateScriptUtil::tagValue(tagValue), name(), error);
                }
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << " SieveActionSetVariable::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveActionSetVariable::code(QWidget *w) const
{
    QString result = QStringLiteral("set ");
    const SelectVariableModifierComboBox *modifier = w->findChild<SelectVariableModifierComboBox *>(QStringLiteral("modifier"));
    const QString modifierStr = modifier->code();
    if (!modifierStr.isEmpty()) {
        result += modifierStr + QLatin1Char(' ');
    }

    if (mHasRegexCapability) {
        const QCheckBox *protectAgainstUseRegexp = w->findChild<QCheckBox *>(QStringLiteral("regexprotect"));
        if (protectAgainstUseRegexp->isChecked()) {
            result += QLatin1String(":quoteregex ");
        }
    }

    const QLineEdit *value = w->findChild<QLineEdit *>(QStringLiteral("value"));
    const QString valueStr = value->text();
    result += QStringLiteral("\"%1\" ").arg(valueStr);

    const QLineEdit *variable = w->findChild<QLineEdit *>(QStringLiteral("variable"));
    const QString variableStr = variable->text();
    result += QStringLiteral("\"%1\";").arg(variableStr);

    return result;
}

QStringList SieveActionSetVariable::needRequires(QWidget *) const
{
    return QStringList() << QStringLiteral("variables");
}

bool SieveActionSetVariable::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionSetVariable::serverNeedsCapability() const
{
    return QStringLiteral("variables");
}

QString SieveActionSetVariable::help() const
{
    QString helpStr = i18n("The \"set\" action stores the specified value in the variable identified by name.");
    if (mHasRegexCapability) {
        helpStr += QLatin1Char('\n') + i18n("This modifier adds the necessary quoting to ensure that the expanded text will only match a literal occurrence if used as a parameter "
                                            "to :regex.  Every character with special meaning (. , *, ? , etc.) is prefixed with \\ in the expansion");
    }
    return helpStr;
}

