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

#include "sieveconditionhasflag.h"
#include "autocreatescripts/commonwidgets/selectmatchtypecombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "autocreatescripts/sieveeditorgraphicalmodewidget.h"
#include "editor/sieveeditorutil.h"

#include <KLocalizedString>
#include <QLineEdit>

#include <QWidget>
#include <QHBoxLayout>
#include "libksieve_debug.h"
#include <QDomNode>
#include <QLabel>

using namespace KSieveUi;
SieveConditionHasFlag::SieveConditionHasFlag(QObject *parent)
    : SieveCondition(QStringLiteral("hasflag"), i18n("Has Flag"), parent)
{
    hasVariableSupport = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QStringLiteral("variables"));
}

SieveCondition *SieveConditionHasFlag::newAction()
{
    return new SieveConditionHasFlag;
}

QWidget *SieveConditionHasFlag::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);
    SelectMatchTypeComboBox *selecttype = new SelectMatchTypeComboBox;
    selecttype->setObjectName(QStringLiteral("matchtype"));
    connect(selecttype, &SelectMatchTypeComboBox::valueChanged, this, &SieveConditionHasFlag::valueChanged);
    lay->addWidget(selecttype);

    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    lay->addLayout(grid);

    int row = 0;
    if (hasVariableSupport) {
        QLabel *lab = new QLabel(i18n("Variable name\n (if empty it uses internal variable):"));
        grid->addWidget(lab, row, 0);

        QLineEdit *variableName = new QLineEdit;
        variableName->setObjectName(QStringLiteral("variablename"));
        connect(variableName, &QLineEdit::textChanged, this, &SieveConditionHasFlag::valueChanged);
        grid->addWidget(variableName, row, 1);
        ++row;
    }
    QLabel *lab = new QLabel(i18n("Value:"));
    grid->addWidget(lab, row, 0);

    QLineEdit *value = new QLineEdit;
    connect(value, &QLineEdit::textChanged, this, &SieveConditionHasFlag::valueChanged);
    value->setObjectName(QStringLiteral("value"));
    grid->addWidget(value, row, 1);

    return w;
}

QString SieveConditionHasFlag::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *matchTypeCombo = w->findChild<SelectMatchTypeComboBox *>(QStringLiteral("matchtype"));
    bool isNegative = false;
    const QString matchString = matchTypeCombo->code(isNegative);

    QString result = AutoCreateScriptUtil::negativeString(isNegative) + QStringLiteral("hasflag %1").arg(matchString);

    if (hasVariableSupport) {
        const QLineEdit *variableName = w->findChild<QLineEdit *>(QStringLiteral("variablename"));
        const QString variableNameStr = variableName->text();
        if (!variableNameStr.isEmpty()) {
            result += QLatin1String(" \"") + variableNameStr + QLatin1Char('"');
        }

        const QLineEdit *value = w->findChild<QLineEdit *>(QStringLiteral("value"));
        const QString valueStr = value->text();
        result += QLatin1String(" \"") + valueStr + QLatin1Char('"');
    }
    return result;
}

QStringList SieveConditionHasFlag::needRequires(QWidget *) const
{
    QStringList lst;
    if (SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QStringLiteral("imap4flags"))) {
        lst << QStringLiteral("imap4flags");
    } else {
        lst << QStringLiteral("imapflags");
    }
    if (hasVariableSupport) {
        lst << QStringLiteral("variables");
    }
    return lst;
}

bool SieveConditionHasFlag::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionHasFlag::serverNeedsCapability() const
{
    if (SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QStringLiteral("imap4flags"))) {
        return QStringLiteral("imap4flags");
    } else {
        return QStringLiteral("imapflags");
    }
}

QString SieveConditionHasFlag::help() const
{
    return i18n("The hasflag test evaluates to true if any of the variables matches any flag name.");
}

bool SieveConditionHasFlag::setParamWidgetValue(const QDomElement &element, QWidget *w, bool notCondition, QString &error)
{
    QStringList strList;
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("tag")) {
                SelectMatchTypeComboBox *matchTypeCombo = w->findChild<SelectMatchTypeComboBox *>(QStringLiteral("matchtype"));
                matchTypeCombo->setCode(AutoCreateScriptUtil::tagValueWithCondition(e.text(), notCondition), name(), error);
            } else if (tagName == QLatin1String("str")) {
                strList << e.text();
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << " SieveConditionExists::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    switch (strList.count()) {
    case 1: {
        QLineEdit *value = w->findChild<QLineEdit *>(QStringLiteral("value"));
        value->setText(strList.at(0));
        break;
    }
    case 2: {
        if (hasVariableSupport) {
            QLineEdit *variableName = w->findChild<QLineEdit *>(QStringLiteral("variablename"));
            variableName->setText(strList.at(0));
            QLineEdit *value = w->findChild<QLineEdit *>(QStringLiteral("value"));
            value->setText(strList.at(1));
        } else {
            qCDebug(LIBKSIEVE_LOG) << " SieveConditionHasFlag has not variable support";
        }
        break;
    }
    default:
        qCDebug(LIBKSIEVE_LOG) << " SieveConditionHasFlag::setParamWidgetValue str list count not correct :" << strList.count();
        break;
    }
    return true;
}

QUrl SieveConditionHasFlag::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

