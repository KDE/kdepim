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

#include "sieveconditionheader.h"
#include "autocreatescripts/commonwidgets/selectmatchtypecombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "widgets/selectheadertypecombobox.h"

#include <KLocalizedString>
#include <QLineEdit>

#include <QHBoxLayout>
#include <QLabel>
#include <QDomNode>
#include "libksieve_debug.h"

using namespace KSieveUi;

SieveConditionHeader::SieveConditionHeader(QObject *parent)
    : SieveCondition(QStringLiteral("header"), i18n("Header"), parent)
{
}

SieveCondition *SieveConditionHeader::newAction()
{
    return new SieveConditionHeader;
}

QWidget *SieveConditionHeader::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectMatchTypeComboBox *matchTypeCombo = new SelectMatchTypeComboBox;
    matchTypeCombo->setObjectName(QStringLiteral("matchtypecombobox"));
    connect(matchTypeCombo, &SelectMatchTypeComboBox::valueChanged, this, &SieveConditionHeader::valueChanged);
    lay->addWidget(matchTypeCombo);

    QGridLayout *grid = new QGridLayout;
    lay->addLayout(grid);

    SelectHeaderTypeComboBox *headerType = new SelectHeaderTypeComboBox;
    headerType->setObjectName(QStringLiteral("headertype"));
    connect(headerType, &SelectHeaderTypeComboBox::valueChanged, this, &SieveConditionHeader::valueChanged);
    grid->addWidget(headerType, 0, 0, 1, 2);

    QLabel *lab = new QLabel(i18n("With value:"));
    grid->addWidget(lab, 1, 0);

    QLineEdit *value = new QLineEdit;
    connect(value, &QLineEdit::textChanged, this, &SieveConditionHeader::valueChanged);
    value->setObjectName(QStringLiteral("value"));
    grid->addWidget(value, 1, 1);
    return w;
}

QString SieveConditionHeader::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *matchTypeCombo = w->findChild<SelectMatchTypeComboBox *>(QStringLiteral("matchtypecombobox"));
    bool isNegative = false;
    const QString matchString = matchTypeCombo->code(isNegative);

    const SelectHeaderTypeComboBox *headerType = w->findChild<SelectHeaderTypeComboBox *>(QStringLiteral("headertype"));
    const QString headerStr = headerType->code();

    const QLineEdit *value = w->findChild<QLineEdit *>(QStringLiteral("value"));
    QString valueStr = value->text();

    valueStr = AutoCreateScriptUtil::fixListValue(valueStr);
    return AutoCreateScriptUtil::negativeString(isNegative) + QStringLiteral("header %1 %2 %3").arg(matchString, headerStr, valueStr);
}

QString SieveConditionHeader::help() const
{
    return i18n("The \"header\" test evaluates to true if the value of any of the named headers, ignoring leading and trailing whitespace, matches any key.");
}

bool SieveConditionHeader::setParamWidgetValue(const QDomElement &element, QWidget *w, bool notCondition, QString &error)
{
    int index = 0;
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("tag")) {
                const QString tagValue = e.text();
                SelectMatchTypeComboBox *selectMatchCombobox = w->findChild<SelectMatchTypeComboBox *>(QStringLiteral("matchtypecombobox"));
                selectMatchCombobox->setCode(AutoCreateScriptUtil::tagValueWithCondition(tagValue, notCondition), name(), error);
            } else if (tagName == QLatin1String("str")) {
                if (index == 0) {
                    SelectHeaderTypeComboBox *headerType = w->findChild<SelectHeaderTypeComboBox *>(QStringLiteral("headertype"));
                    headerType->setCode(e.text());
                } else if (index == 1) {
                    QLineEdit *value = w->findChild<QLineEdit *>(QStringLiteral("value"));
                    value->setText(e.text().replace(QStringLiteral("\""), QStringLiteral("\\\"")));
                } else {
                    tooManyArgument(tagName, index, 2, error);
                    qCDebug(LIBKSIEVE_LOG) << " SieveConditionHeader::setParamWidgetValue too many argument " << index;
                }
                ++index;
            } else if (tagName == QLatin1String("list")) {
                //Header list
                if (index == 0) {
                    SelectHeaderTypeComboBox *headerType = w->findChild<SelectHeaderTypeComboBox *>(QStringLiteral("headertype"));
                    headerType->setCode(AutoCreateScriptUtil::listValueToStr(e));
                } else if (index == 1) {
                    QLineEdit *value = w->findChild<QLineEdit *>(QStringLiteral("value"));
                    value->setText(AutoCreateScriptUtil::listValueToStr(e));
                } else {
                    tooManyArgument(tagName, index, 2, error);
                    qCDebug(LIBKSIEVE_LOG) << " SieveConditionHeader::setParamWidgetValue too many argument " << index;
                }
                ++index;
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << " SieveConditionHeader::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

