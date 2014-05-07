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

#include "sieveconditionhasflag.h"
#include "autocreatescripts/commonwidgets/selectmatchtypecombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "autocreatescripts/sieveeditorgraphicalmodewidget.h"
#include "editor/sieveeditorutil.h"

#include <KLocalizedString>
#include <QLineEdit>

#include <QWidget>
#include <QHBoxLayout>
#include <QDebug>
#include <QDomNode>
#include <QLabel>

using namespace KSieveUi;
SieveConditionHasFlag::SieveConditionHasFlag(QObject *parent)
    : SieveCondition(QLatin1String("hasflag"), i18n("Has Flag"), parent)
{
    hasVariableSupport = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QLatin1String("variables"));
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
    connect(selecttype, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
    lay->addWidget(selecttype);


    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    lay->addLayout(grid);

    QLabel *lab = new QLabel(i18n("Variable name\n (if empty it uses internal variable):"));
    grid->addWidget(lab, 0, 0);

    if (hasVariableSupport) {
        QLineEdit *variableName = new QLineEdit;
        variableName->setObjectName(QLatin1String("variablename"));
        connect(variableName, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));
        grid->addWidget(variableName, 0, 1);
    }
    lab = new QLabel(i18n("Value:"));
    grid->addWidget(lab, 1, 0);

    QLineEdit *value = new QLineEdit;
    connect(value, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));
    value->setObjectName(QLatin1String("value"));
    grid->addWidget(value, 1, 1);

    return w;
}

QString SieveConditionHasFlag::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *matchTypeCombo = w->findChild<SelectMatchTypeComboBox*>( QLatin1String("matchtype") );
    bool isNegative = false;
    const QString matchString = matchTypeCombo->code(isNegative);

    QString result = AutoCreateScriptUtil::negativeString(isNegative) + QString::fromLatin1("hasflag %1").arg(matchString);

    if (hasVariableSupport) {
        const QLineEdit *variableName = w->findChild<QLineEdit*>(QLatin1String("variablename"));
        const QString variableNameStr = variableName->text();
        if (!variableNameStr.isEmpty()) {
            result += QLatin1String(" \"") + variableNameStr + QLatin1Char('"');
        }

        const QLineEdit *value = w->findChild<QLineEdit*>(QLatin1String("value"));
        const QString valueStr = value->text();
        result += QLatin1String(" \"") + valueStr + QLatin1Char('"');
    }
    return result;
}

QStringList SieveConditionHasFlag::needRequires(QWidget *) const
{
    QStringList lst;
    if (SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QLatin1String("imap4flags")))
        lst << QLatin1String("imap4flags");
    else
        lst << QLatin1String("imapflags");
    if (hasVariableSupport)
        lst << QLatin1String("variables");
    return lst;
}

bool SieveConditionHasFlag::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionHasFlag::serverNeedsCapability() const
{
    if (SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QLatin1String("imap4flags")))
        return QLatin1String("imap4flags");
    else
        return QLatin1String("imapflags");
}

QString SieveConditionHasFlag::help() const
{
    return i18n("The hasflag test evaluates to true if any of the variables matches any flag name.");
}

bool SieveConditionHasFlag::setParamWidgetValue(const QDomElement &element, QWidget *w, bool notCondition , QString &error)
{
    QStringList strList;
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("tag")) {
                SelectMatchTypeComboBox *matchTypeCombo = w->findChild<SelectMatchTypeComboBox*>( QLatin1String("matchtype") );
                matchTypeCombo->setCode(AutoCreateScriptUtil::tagValueWithCondition(e.text(), notCondition), name(), error);
            } else if (tagName == QLatin1String("str")) {
                strList << e.text();
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qDebug()<<" SieveConditionExists::setParamWidgetValue unknown tagName "<<tagName;
            }
        }
        node = node.nextSibling();
    }
    switch (strList.count()) {
    case 1: {
        QLineEdit *value = w->findChild<QLineEdit*>(QLatin1String("value"));
        value->setText(strList.at(0));
        break;
    }
    case 2: {
        if (hasVariableSupport) {
            QLineEdit *variableName = w->findChild<QLineEdit*>(QLatin1String("variablename"));
            variableName->setText(strList.at(0));
            QLineEdit *value = w->findChild<QLineEdit*>(QLatin1String("value"));
            value->setText(strList.at(1));
        } else {
            qDebug()<<" SieveConditionHasFlag has not variable support";
        }
        break;
    }
    default:
        qDebug()<<" SieveConditionHasFlag::setParamWidgetValue str list count not correct :"<<strList.count();
        break;
    }
    return true;
}

QString SieveConditionHasFlag::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}



