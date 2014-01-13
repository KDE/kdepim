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


#include "sieveactionnotify.h"
#include "editor/sieveeditorutil.h"
#include "widgets/selectimportancecombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLocalizedString>
#include <KLineEdit>


#include <QHBoxLayout>
#include <QLabel>
#include <QDomNode>
#include <QDebug>

using namespace KSieveUi;

SieveActionNotify::SieveActionNotify(QObject *parent)
    : SieveAction(QLatin1String("notify"), i18n("Notify"), parent)
{
}

SieveAction* SieveActionNotify::newAction()
{
    return new SieveActionNotify;
}

QWidget *SieveActionNotify::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectImportanceCombobox *importanceCombobox = new SelectImportanceCombobox;
    importanceCombobox->setObjectName(QLatin1String("importancecombo"));
    connect(importanceCombobox, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
    lay->addWidget(importanceCombobox);

    QLabel *lab = new QLabel(i18n("message:"));
    lay->addWidget(lab);

    KLineEdit *message = new KLineEdit;
    message->setObjectName(QLatin1String("message"));
    connect(message, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));
    lay->addWidget(message);
    connect(message, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));

    lab = new QLabel(i18n("method:"));
    lay->addWidget(lab);

    KLineEdit *method = new KLineEdit;
    method->setObjectName(QLatin1String("method"));
    connect(method, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));
    lay->addWidget(method);
    connect(method, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));


    return w;
}

bool SieveActionNotify::setParamWidgetValue(const QDomElement &element, QWidget *w, QString &error )
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("tag")) {
                const QString tagValue = e.text();
                if (tagValue == QLatin1String("message")) {
                    const QString strValue = AutoCreateScriptUtil::strValue(node);
                    if (!strValue.isEmpty()) {
                        KLineEdit *message = w->findChild<KLineEdit*>( QLatin1String("message") );
                        message->setText(AutoCreateScriptUtil::quoteStr(strValue));
                    }
                } else if (tagValue == QLatin1String("importance")) {
                    const QString strValue = AutoCreateScriptUtil::strValue(node);
                    if (!strValue.isEmpty()) {
                        SelectImportanceCombobox *importance = w->findChild<SelectImportanceCombobox*>( QLatin1String("importancecombo") );
                        importance->setCode(strValue, name(), error);
                    }
                } else {
                    unknowTagValue(tagValue, error);
                    qDebug()<<" SieveActionNotify::setParamWidgetValue unknown tagValue"<<tagValue;
                }
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else if (tagName == QLatin1String("str")) {
                KLineEdit *method = w->findChild<KLineEdit*>( QLatin1String("method") );
                method->setText(AutoCreateScriptUtil::quoteStr(e.text()));
            } else {
                unknownTag(tagName, error);
                qDebug()<<" SieveActionNotify::setParamWidgetValue unknown tagName "<<tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveActionNotify::code(QWidget *w) const
{
    QString result = QLatin1String("notify");
    const SelectImportanceCombobox *importance = w->findChild<SelectImportanceCombobox*>( QLatin1String("importancecombo") );
    const QString importanceStr = importance->code();
    if (!importanceStr.isEmpty()) {
        result += QString::fromLatin1(" :importance \"%1\"").arg(importanceStr);
    }

    const KLineEdit *message = w->findChild<KLineEdit*>( QLatin1String("message") );
    const QString messageStr = message->text();
    if (!messageStr.isEmpty()) {
        result += QString::fromLatin1(" :message \"%2\"").arg(messageStr);
    }

    const KLineEdit *method = w->findChild<KLineEdit*>( QLatin1String("method") );
    const QString methodStr = method->text();
    result += QString::fromLatin1(" \"%3\";").arg(methodStr);

    return result;
}

QString SieveActionNotify::serverNeedsCapability() const
{
    return QLatin1String("enotify");
}

bool SieveActionNotify::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionNotify::help() const
{
    return i18n("The \"notify\" action specifies that a notification should be sent to a user.");
}

QStringList SieveActionNotify::needRequires(QWidget *) const
{
    QStringList lst;
    lst << QLatin1String("enotify");
    return lst;
}

QString SieveActionNotify::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

