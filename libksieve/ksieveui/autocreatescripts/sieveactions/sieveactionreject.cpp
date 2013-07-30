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

#include "sieveactionreject.h"
#include "widgets/multilineedit.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLocale>

#include <QLabel>
#include <QHBoxLayout>
#include <QDomNode>
#include <QDebug>

using namespace KSieveUi;
SieveActionReject::SieveActionReject(QObject *parent)
    : SieveAction(QLatin1String("reject"), i18n("Reject"), parent)
{
}

SieveAction *SieveActionReject::newAction()
{
    return new SieveActionReject;
}

QWidget *SieveActionReject::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);
    QLabel *lab = new QLabel(i18n("text:"));
    lay->addWidget(lab);

    MultiLineEdit *edit = new MultiLineEdit;
    edit->setObjectName( QLatin1String("rejectmessage") );
    lay->addWidget(edit);
    return w;
}

void SieveActionReject::setParamWidgetValue(const QDomElement &element, QWidget *w )
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                const QString tagValue = e.text();
                MultiLineEdit *edit = w->findChild<MultiLineEdit*>( QLatin1String("rejectmessage") );
                edit->setText(AutoCreateScriptUtil::quoteStr(tagValue));
            } else {
                qDebug()<<" SieveActionReject::setParamWidgetValue unknown tagName "<<tagName;
            }
        }
        node = node.nextSibling();
    }
}

QString SieveActionReject::code(QWidget *w) const
{
    const MultiLineEdit *edit = w->findChild<MultiLineEdit*>( QLatin1String("rejectmessage") );
    const QString text = edit->toPlainText();

    return QString::fromLatin1("reject text:%1").arg(AutoCreateScriptUtil::createMultiLine(text));
}

QStringList SieveActionReject::needRequires(QWidget *) const
{
    return QStringList() << QLatin1String("reject");
}

QString SieveActionReject::serverNeedsCapability() const
{
    return QLatin1String("reject");
}

bool SieveActionReject::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionReject::help() const
{
    return i18n(" The \"reject\" action cancels the implicit keep and refuses delivery of a message.");
}

#include "sieveactionreject.moc"
