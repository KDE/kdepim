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

#include "sieveconditionservermetadata.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "autocreatescripts/commonwidgets/selectmatchtypecombobox.h"
#include "editor/sieveeditorutil.h"

#include <KLocalizedString>
#include <QLineEdit>

#include <QWidget>
#include <QHBoxLayout>
#include "libksieve_debug.h"
#include <QLabel>
#include <QDomNode>

using namespace KSieveUi;
SieveConditionServerMetaData::SieveConditionServerMetaData(QObject *parent)
    : SieveCondition(QStringLiteral("servermetadata"), i18n("Server Meta Data"), parent)
{
}

SieveCondition *SieveConditionServerMetaData::newAction()
{
    return new SieveConditionServerMetaData;
}

QWidget *SieveConditionServerMetaData::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectMatchTypeComboBox *selectType = new SelectMatchTypeComboBox;
    selectType->setObjectName(QStringLiteral("selecttype"));
    connect(selectType, &SelectMatchTypeComboBox::valueChanged, this, &SieveConditionServerMetaData::valueChanged);
    lay->addWidget(selectType);

    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    lay->addLayout(grid);

    QLabel *lab = new QLabel(i18n("Mailbox:"));
    grid->addWidget(lab, 0, 0);

    QLineEdit *mailbox = new QLineEdit;
    mailbox->setObjectName(QStringLiteral("mailbox"));
    connect(mailbox, &QLineEdit::textChanged, this, &SieveConditionServerMetaData::valueChanged);
    grid->addWidget(mailbox, 0, 1);

    lab = new QLabel(i18n("Annotations:"));
    grid->addWidget(lab, 1, 0);

    QLineEdit *annotation = new QLineEdit;
    connect(annotation, &QLineEdit::textChanged, this, &SieveConditionServerMetaData::valueChanged);
    annotation->setObjectName(QStringLiteral("annotation"));
    grid->addWidget(annotation, 1, 1);

    lab = new QLabel(i18n("Value:"));
    grid->addWidget(lab, 2, 0);

    QLineEdit *value = new QLineEdit;
    connect(value, &QLineEdit::textChanged, this, &SieveConditionServerMetaData::valueChanged);
    value->setObjectName(QStringLiteral("value"));
    grid->addWidget(value, 2, 1);

    return w;
}

QString SieveConditionServerMetaData::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *selectType = w->findChild<SelectMatchTypeComboBox *>(QStringLiteral("selecttype"));
    bool isNegative = false;
    const QString matchString = selectType->code(isNegative);

    QString result = (isNegative ? QStringLiteral("not ") : QString()) + QStringLiteral("servermetadata %1 ").arg(matchString);

    const QLineEdit *mailbox = w->findChild<QLineEdit *>(QStringLiteral("mailbox"));
    const QString mailboxStr = mailbox->text();

    result += QStringLiteral("\"%1\" ").arg(mailboxStr);

    const QLineEdit *annotation = w->findChild<QLineEdit *>(QStringLiteral("annotation"));
    const QString annotationStr = annotation->text();

    result += QStringLiteral("\"%1\" ").arg(annotationStr);

    const QLineEdit *value = w->findChild<QLineEdit *>(QStringLiteral("value"));
    const QString valueStr = value->text();

    result += QStringLiteral("\"%1\"").arg(valueStr);
    return result;
}

QStringList SieveConditionServerMetaData::needRequires(QWidget *) const
{
    return QStringList() << QStringLiteral("servermetadata");
}

bool SieveConditionServerMetaData::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionServerMetaData::serverNeedsCapability() const
{
    return QStringLiteral("servermetadata");
}

QString SieveConditionServerMetaData::help() const
{
    return i18n("This test retrieves the value of the server annotation \"annotation-name\".  The retrieved value is compared to the \"key-list\". The test returns true if the annotation exists and its value matches any of the keys.");
}

bool SieveConditionServerMetaData::setParamWidgetValue(const QDomElement &element, QWidget *w, bool notCondition, QString &error)
{
    int index = 0;
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                const QString tagValue = e.text();
                switch (index) {
                case 0: {
                    QLineEdit *mailbox = w->findChild<QLineEdit *>(QStringLiteral("mailbox"));
                    mailbox->setText(tagValue);
                    break;
                }
                case 1: {
                    QLineEdit *annotation = w->findChild<QLineEdit *>(QStringLiteral("annotation"));
                    annotation->setText(tagValue);
                    break;
                }
                case 2: {
                    QLineEdit *value = w->findChild<QLineEdit *>(QStringLiteral("value"));
                    value->setText(tagValue);
                    break;
                }
                default: {
                    tooManyArgument(tagName, index, 3, error);
                    qCDebug(LIBKSIEVE_LOG) << " SieveConditionServerMetaData::setParamWidgetValue too many argument " << index;
                    break;
                }
                }
                ++index;
            } else if (tagName == QLatin1String("tag")) {
                SelectMatchTypeComboBox *selectType = w->findChild<SelectMatchTypeComboBox *>(QStringLiteral("selecttype"));
                selectType->setCode(AutoCreateScriptUtil::tagValueWithCondition(e.text(), notCondition), name(), error);
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << " SieveConditionServerMetaData::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QUrl SieveConditionServerMetaData::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}
