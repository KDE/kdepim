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


#include "sieveactionenclose.h"
#include "widgets/multilineedit.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "editor/sieveeditorutil.h"

#include <KLocalizedString>
#include <KLineEdit>

#include <QLabel>
#include <QDomNode>
#include <QDebug>
#include <QGridLayout>

using namespace KSieveUi;
SieveActionEnclose::SieveActionEnclose(QObject *parent)
    : SieveAction(QLatin1String("enclose"), i18n("Enclose"), parent)
{
}

SieveAction* SieveActionEnclose::newAction()
{
    return new SieveActionEnclose;
}

QWidget *SieveActionEnclose::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    w->setLayout(grid);

    QLabel *lab = new QLabel(i18n("Subject:"));
    grid->addWidget(lab,0,0);

    KLineEdit *subject = new KLineEdit;
    subject->setObjectName(QLatin1String("subject"));
    grid->addWidget(subject,0,1);

    lab = new QLabel(i18n("headers:"));
    grid->addWidget(lab,1,0);

    KLineEdit *headers = new KLineEdit;
    headers->setObjectName(QLatin1String("headers"));
    grid->addWidget(headers,1,1);

    lab = new QLabel(i18n("text:"));
    grid->addWidget(lab,2,0);

    MultiLineEdit *text = new MultiLineEdit;
    text->setObjectName(QLatin1String("text"));
    grid->addWidget(text,2,1);

    return w;
}

bool SieveActionEnclose::setParamWidgetValue(const QDomElement &element, QWidget *w, QString &error )
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("tag")) {
                const QString tagValue = e.text();
                if (tagValue == QLatin1String("headers")) {
                    const QString strValue = AutoCreateScriptUtil::strValue(node);
                    if (!strValue.isEmpty()) {
                        KLineEdit *subject = w->findChild<KLineEdit*>(QLatin1String("subject"));
                        subject->setText(strValue);
                    }
                } else if (tagValue == QLatin1String("subject")) {
                    const QString strValue = AutoCreateScriptUtil::strValue(node);
                    if (!strValue.isEmpty()) {
                        KLineEdit *headers = w->findChild<KLineEdit*>(QLatin1String("headers"));
                        headers->setText(strValue);
                    }
                } else {
                    unknowTagValue(tagValue, error);
                    qDebug()<<" SieveActionEnclose::setParamWidgetValue unknown tag value:"<<tagValue;
                }
            } else if (tagName == QLatin1String("str")) {
                MultiLineEdit *edit = w->findChild<MultiLineEdit*>( QLatin1String("text") );
                edit->setText(e.text());
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qDebug()<<" SieveActionEnclose::setParamWidgetValue unknown tagName "<<tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveActionEnclose::code(QWidget *w) const
{
    QString result = QLatin1String("enclose ");
    const KLineEdit *subject = w->findChild<KLineEdit*>(QLatin1String("subject"));
    const QString subjectStr = subject->text();
    if (!subjectStr.isEmpty()) {
        result += QString::fromLatin1(":subject \"%1\" ").arg(subjectStr);
    }

    const KLineEdit *headers = w->findChild<KLineEdit*>(QLatin1String("headers"));
    const QString headersStr = headers->text();
    if (!headersStr.isEmpty()) {
        result += QString::fromLatin1(":headers \"%1\" ").arg(headersStr);
    }

    const MultiLineEdit *edit = w->findChild<MultiLineEdit*>( QLatin1String("text") );
    const QString text = edit->toPlainText();
    if (!text.isEmpty()) {
        result += QString::fromLatin1("text:%1").arg(AutoCreateScriptUtil::createMultiLine(text));
    } else {
        result += QLatin1Char(';');
    }

    return result;
}


QStringList SieveActionEnclose::needRequires(QWidget */*parent*/) const
{
    return QStringList() << QLatin1String("enclose");
}

bool SieveActionEnclose::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionEnclose::serverNeedsCapability() const
{
    return QLatin1String("enclose");
}

QString SieveActionEnclose::help() const
{
    return i18n("Enclose action command is defined to allow an entire message to be enclosed as an attachment to a new message.");
}

QString SieveActionEnclose::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

