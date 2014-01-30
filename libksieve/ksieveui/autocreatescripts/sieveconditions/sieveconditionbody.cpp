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

#include "sieveconditionbody.h"
#include "widgets/selectbodytypewidget.h"
#include "autocreatescripts/commonwidgets/selectmatchtypecombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "editor/sieveeditorutil.h"

#include <KLocalizedString>
#include <KLineEdit>

#include <QWidget>
#include <QHBoxLayout>
#include <QDebug>

using namespace KSieveUi;
SieveConditionBody::SieveConditionBody(QObject *parent)
    : SieveCondition(QLatin1String("body"), i18n("Body"), parent)
{
}

SieveCondition *SieveConditionBody::newAction()
{
    return new SieveConditionBody;
}

QWidget *SieveConditionBody::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectBodyTypeWidget *bodyType = new SelectBodyTypeWidget;
    bodyType->setObjectName(QLatin1String("bodytype"));
    connect(bodyType, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
    lay->addWidget(bodyType);

    SelectMatchTypeComboBox *matchType = new SelectMatchTypeComboBox;
    lay->addWidget(matchType);
    matchType->setObjectName(QLatin1String("matchtype"));
    connect(matchType, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));

    KLineEdit *edit = new KLineEdit;
    connect(edit, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));
    edit->setClearButtonShown(true);
    lay->addWidget(edit);
    edit->setObjectName(QLatin1String("edit"));

    return w;
}

QString SieveConditionBody::code(QWidget *w) const
{
    const SelectBodyTypeWidget *bodyType =  w->findChild<SelectBodyTypeWidget*>( QLatin1String("bodytype") );
    const QString bodyValue = bodyType->code();
    const SelectMatchTypeComboBox *matchType = w->findChild<SelectMatchTypeComboBox*>( QLatin1String("matchtype"));
    bool isNegative = false;
    const QString matchValue = matchType->code(isNegative);

    KLineEdit *edit = w->findChild<KLineEdit*>( QLatin1String("edit"));
    const QString editValue = edit->text();
    return AutoCreateScriptUtil::negativeString(isNegative) + QString::fromLatin1("body %1 %2 \"%3\"").arg(bodyValue).arg(matchValue).arg(editValue);
}

QStringList SieveConditionBody::needRequires(QWidget *) const
{
    return QStringList() << QLatin1String("body");
}

bool SieveConditionBody::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionBody::serverNeedsCapability() const
{
    return QLatin1String("body");
}

QString SieveConditionBody::help() const
{
    return i18n("The body test matches content in the body of an email message, that is, anything following the first empty line after the header.  (The empty line itself, if present, is not considered to be part of the body.)");
}

bool SieveConditionBody::setParamWidgetValue(const QDomElement &element, QWidget *w, bool notCondition , QString &error)
{
    int index = 0;
    int indexStr = 0;
    QStringList tagValueList;
    QStringList strValue;

    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("tag")) {
                const QString tagValue = e.text();
                if (index == 0) {
                    tagValueList<<AutoCreateScriptUtil::tagValue(tagValue);
                } else if (index == 1) {
                    tagValueList<<AutoCreateScriptUtil::tagValue(AutoCreateScriptUtil::tagValueWithCondition(e.text(), notCondition));
                } else {
                    tooManyArgument(tagName, index, 2, error);
                    qDebug()<<" SieveConditionBody::setParamWidgetValue too many argument "<<index;
                }
                ++index;
            } else if (tagName == QLatin1String("str")) {
                strValue<<e.text();
                ++indexStr;
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qDebug()<<" SieveConditionBody::setParamWidgetValue unknown tagName "<<tagName;
            }
        }
        node = node.nextSibling();
    }

    if (strValue.count() == 1) {
        SelectBodyTypeWidget *bodyType =  w->findChild<SelectBodyTypeWidget*>( QLatin1String("bodytype") );
        bodyType->setCode(tagValueList.at(0), QString(), name(), error);
        SelectMatchTypeComboBox *matchType = w->findChild<SelectMatchTypeComboBox*>( QLatin1String("matchtype"));
        matchType->setCode(strValue.at(0), name(), error);
        KLineEdit *edit = w->findChild<KLineEdit*>( QLatin1String("edit"));
        edit->setText(AutoCreateScriptUtil::quoteStr(strValue.at(0)));
    } else if (strValue.count() == 2) {
        SelectBodyTypeWidget *bodyType =  w->findChild<SelectBodyTypeWidget*>( QLatin1String("bodytype") );
        bodyType->setCode(tagValueList.at(0), indexStr == 2 ? strValue.at(0) : QString(), name(), error);
        SelectMatchTypeComboBox *matchType = w->findChild<SelectMatchTypeComboBox*>( QLatin1String("matchtype"));
        matchType->setCode(tagValueList.at(1), name(), error);
        KLineEdit *edit = w->findChild<KLineEdit*>( QLatin1String("edit"));
        edit->setText(indexStr == 1 ? AutoCreateScriptUtil::quoteStr(strValue.at(0)) : AutoCreateScriptUtil::quoteStr(strValue.at(1)));
    }
    return true;
}

QString SieveConditionBody::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}


