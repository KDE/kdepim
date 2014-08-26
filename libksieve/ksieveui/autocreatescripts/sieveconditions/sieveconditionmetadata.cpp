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

#include "sieveconditionmetadata.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "autocreatescripts/commonwidgets/selectmatchtypecombobox.h"
#include "editor/sieveeditorutil.h"
#include <KLocalizedString>
#include <QLineEdit>

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>
#include <QDomNode>

using namespace KSieveUi;
SieveConditionMetaData::SieveConditionMetaData(QObject *parent)
    : SieveCondition(QLatin1String("metadata"), i18n("Meta Data"), parent)
{
}

SieveCondition *SieveConditionMetaData::newAction()
{
    return new SieveConditionMetaData;
}

QWidget *SieveConditionMetaData::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectMatchTypeComboBox *selectType = new SelectMatchTypeComboBox;
    selectType->setObjectName(QLatin1String("selecttype"));
    connect(selectType, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
    lay->addWidget(selectType);

    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    lay->addLayout(grid);

    QLabel *lab = new QLabel(i18n("Mailbox:"));
    grid->addWidget(lab, 0, 0);

    QLineEdit *mailbox = new QLineEdit;
    connect(mailbox, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));
    mailbox->setObjectName(QLatin1String("mailbox"));
    grid->addWidget(mailbox, 0, 1);

    lab = new QLabel(i18n("Annotations:"));
    grid->addWidget(lab, 1, 0);

    QLineEdit *annotation = new QLineEdit;
    connect(annotation, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));
    annotation->setObjectName(QLatin1String("annotation"));
    grid->addWidget(annotation, 1, 1);

    lab = new QLabel(i18n("Value:"));
    grid->addWidget(lab, 2, 0);

    QLineEdit *value = new QLineEdit;
    connect(value, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));
    value->setObjectName(QLatin1String("value"));
    grid->addWidget(value, 2, 1);

    return w;
}

QString SieveConditionMetaData::code(QWidget *w) const
{
    const SelectMatchTypeComboBox *selectType = w->findChild<SelectMatchTypeComboBox *>(QLatin1String("selecttype"));
    bool isNegative = false;
    const QString matchString = selectType->code(isNegative);

    QString result = AutoCreateScriptUtil::negativeString(isNegative) + QString::fromLatin1("metadata %1 ").arg(matchString);

    const QLineEdit *mailbox = w->findChild<QLineEdit *>(QLatin1String("mailbox"));
    const QString mailboxStr = mailbox->text();

    result += QString::fromLatin1("\"%1\" ").arg(mailboxStr);

    const QLineEdit *annotation = w->findChild<QLineEdit *>(QLatin1String("annotation"));
    const QString annotationStr = annotation->text();

    result += QString::fromLatin1("\"%1\" ").arg(annotationStr);

    const QLineEdit *value = w->findChild<QLineEdit *>(QLatin1String("value"));
    const QString valueStr = value->text();

    result += QString::fromLatin1("\"%1\"").arg(valueStr);
    return result;
}

QStringList SieveConditionMetaData::needRequires(QWidget *) const
{
    return QStringList() << QLatin1String("mboxmetadata");
}

bool SieveConditionMetaData::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionMetaData::serverNeedsCapability() const
{
    return QLatin1String("mboxmetadata");
}

QString SieveConditionMetaData::help() const
{
    return i18n("This test retrieves the value of the mailbox annotation \"annotation-name\" for the mailbox \"mailbox\". The retrieved value is compared to the \"key-list\". The test returns true if the annotation exists and its value matches any of the keys.");
}

bool SieveConditionMetaData::setParamWidgetValue(const QDomElement &element, QWidget *w, bool notCondition, QString &error)
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
                    QLineEdit *mailbox = w->findChild<QLineEdit *>(QLatin1String("mailbox"));
                    mailbox->setText(tagValue);
                    break;
                }
                case 1: {
                    QLineEdit *annotation = w->findChild<QLineEdit *>(QLatin1String("annotation"));
                    annotation->setText(AutoCreateScriptUtil::quoteStr(tagValue));
                    break;
                }
                case 2: {
                    QLineEdit *value = w->findChild<QLineEdit *>(QLatin1String("value"));
                    value->setText(AutoCreateScriptUtil::quoteStr(tagValue));
                    break;
                }
                default: {
                    tooManyArgument(tagName, index, 3, error);
                    qDebug() << " SieveConditionMetaData::setParamWidgetValue too many argument " << index;
                    break;
                }
                }
                ++index;
            } else if (tagName == QLatin1String("tag")) {
                SelectMatchTypeComboBox *selectType = w->findChild<SelectMatchTypeComboBox *>(QLatin1String("selecttype"));
                selectType->setCode(AutoCreateScriptUtil::tagValueWithCondition(e.text(), notCondition), name(), error);
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qDebug() << " SieveConditionMetaData::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveConditionMetaData::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

