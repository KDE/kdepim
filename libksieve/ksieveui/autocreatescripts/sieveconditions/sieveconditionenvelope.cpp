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

#include "sieveconditionenvelope.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include "widgets/selectaddresspartcombobox.h"
#include "autocreatescripts/commonwidgets/selectmatchtypecombobox.h"
#include "widgets/selectheadertypecombobox.h"
#include "editor/sieveeditorutil.h"

#include <QLineEdit>
#include <KLocalizedString>

#include <QHBoxLayout>
#include <QLabel>
#include "libksieve_debug.h"
#include <QDomNode>

using namespace KSieveUi;

SieveConditionEnvelope::SieveConditionEnvelope(QObject *parent)
    : SieveCondition(QStringLiteral("envelope"), i18n("Envelope"), parent)
{
}

SieveCondition *SieveConditionEnvelope::newAction()
{
    return new SieveConditionEnvelope;
}

QWidget *SieveConditionEnvelope::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectAddressPartComboBox *selectAddressPart = new SelectAddressPartComboBox;
    connect(selectAddressPart, &SelectAddressPartComboBox::valueChanged, this, &SieveConditionEnvelope::valueChanged);
    selectAddressPart->setObjectName(QStringLiteral("addresspartcombobox"));
    lay->addWidget(selectAddressPart);

    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    lay->addLayout(grid);

    SelectMatchTypeComboBox *selectMatchCombobox = new SelectMatchTypeComboBox;
    selectMatchCombobox->setObjectName(QStringLiteral("matchtypecombobox"));
    connect(selectMatchCombobox, &SelectMatchTypeComboBox::valueChanged, this, &SieveConditionEnvelope::valueChanged);
    grid->addWidget(selectMatchCombobox, 0, 0);

    SelectHeaderTypeComboBox *selectHeaderType = new SelectHeaderTypeComboBox(true);
    selectHeaderType->setObjectName(QStringLiteral("headertypecombobox"));
    connect(selectHeaderType, &SelectHeaderTypeComboBox::valueChanged, this, &SieveConditionEnvelope::valueChanged);
    grid->addWidget(selectHeaderType, 0, 1);

    QLabel *lab = new QLabel(i18n("address:"));
    grid->addWidget(lab, 1, 0);

    QLineEdit *edit = new QLineEdit;
    connect(edit, &QLineEdit::textChanged, this, &SieveConditionEnvelope::valueChanged);
    edit->setClearButtonEnabled(true);
    edit->setPlaceholderText(i18n("Use ; to separate emails"));
    grid->addWidget(edit, 1, 1);
    edit->setObjectName(QStringLiteral("editaddress"));

    return w;
}

QString SieveConditionEnvelope::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *selectMatchCombobox = w->findChild<SelectMatchTypeComboBox *>(QStringLiteral("matchtypecombobox"));
    bool isNegative = false;
    const QString matchTypeStr = selectMatchCombobox->code(isNegative);

    const SelectAddressPartComboBox *selectAddressPart = w->findChild<SelectAddressPartComboBox *>(QStringLiteral("addresspartcombobox"));
    const QString selectAddressPartStr = selectAddressPart->code();

    const SelectHeaderTypeComboBox *selectHeaderType = w->findChild<SelectHeaderTypeComboBox *>(QStringLiteral("headertypecombobox"));
    const QString selectHeaderTypeStr = selectHeaderType->code();

    const QLineEdit *edit = w->findChild<QLineEdit *>(QStringLiteral("editaddress"));
    const QString addressStr = AutoCreateScriptUtil::createAddressList(edit->text().trimmed(), false);
    return AutoCreateScriptUtil::negativeString(isNegative) + QString::fromLatin1("envelope %1 %2 %3 %4").arg(selectAddressPartStr).arg(matchTypeStr).arg(selectHeaderTypeStr).arg(addressStr);
}

QStringList SieveConditionEnvelope::needRequires(QWidget *w) const
{
    const SelectAddressPartComboBox *selectAddressPart = w->findChild<SelectAddressPartComboBox *>(QStringLiteral("addresspartcombobox"));
    return QStringList() << QStringLiteral("envelope") << selectAddressPart->extraRequire();
}

bool SieveConditionEnvelope::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionEnvelope::serverNeedsCapability() const
{
    return QStringLiteral("envelope");
}

QString SieveConditionEnvelope::help() const
{
    return i18n("The \"envelope\" test is true if the specified part of the [SMTP] (or equivalent) envelope matches the specified key.");
}

bool SieveConditionEnvelope::setParamWidgetValue(const QDomElement &element, QWidget *w, bool notCondition , QString &error)
{
    int indexTag = 0;
    int indexStr = 0;
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QStringLiteral("tag")) {
                const QString tagValue = e.text();
                if (indexTag == 0) {
                    SelectAddressPartComboBox *selectAddressPart = w->findChild<SelectAddressPartComboBox *>(QStringLiteral("addresspartcombobox"));
                    selectAddressPart->setCode(AutoCreateScriptUtil::tagValue(tagValue), name(), error);
                } else if (indexTag == 1) {
                    SelectMatchTypeComboBox *selectMatchCombobox = w->findChild<SelectMatchTypeComboBox *>(QStringLiteral("matchtypecombobox"));
                    selectMatchCombobox->setCode(AutoCreateScriptUtil::tagValueWithCondition(tagValue, notCondition), name(), error);
                } else {
                    tooManyArgument(tagName, indexTag, 2, error);
                    qCDebug(LIBKSIEVE_LOG) << "SieveConditionEnvelope::setParamWidgetValue too many argument :" << indexTag;
                }
                ++indexTag;
            } else if (tagName == QStringLiteral("str")) {
                if (indexStr == 0) {
                    SelectHeaderTypeComboBox *selectHeaderType = w->findChild<SelectHeaderTypeComboBox *>(QStringLiteral("headertypecombobox"));
                    selectHeaderType->setCode(e.text());
                } else if (indexStr == 1) {
                    QLineEdit *edit = w->findChild<QLineEdit *>(QStringLiteral("editaddress"));
                    edit->setText(AutoCreateScriptUtil::quoteStr(e.text()));
                } else {
                    tooManyArgument(tagName, indexStr, 2, error);
                    qCDebug(LIBKSIEVE_LOG) << "SieveConditionEnvelope::setParamWidgetValue too many argument indexStr " << indexStr;
                }
                ++indexStr;
            } else if (tagName == QStringLiteral("list")) {
                if (indexStr == 0) {
                    SelectHeaderTypeComboBox *selectHeaderType = w->findChild<SelectHeaderTypeComboBox *>(QStringLiteral("headertypecombobox"));
                    selectHeaderType->setCode(AutoCreateScriptUtil::listValueToStr(e));
                } else if (indexStr == 1) {
                    QLineEdit *edit = w->findChild<QLineEdit *>(QStringLiteral("editaddress"));
                    edit->setText(AutoCreateScriptUtil::listValueToStr(e));
                }
                ++indexStr;
            } else if (tagName == QStringLiteral("crlf")) {
                //nothing
            } else if (tagName == QStringLiteral("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << " SieveConditionEnvelope::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString KSieveUi::SieveConditionEnvelope::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}
