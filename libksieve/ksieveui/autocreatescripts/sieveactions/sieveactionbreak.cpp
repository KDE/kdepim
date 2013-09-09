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

#include "sieveactionbreak.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLocale>
#include <KLineEdit>

#include <QHBoxLayout>
#include <QLabel>
#include <QDomNode>
#include <QDebug>

using namespace KSieveUi;
SieveActionBreak::SieveActionBreak(QObject *parent)
    : SieveAction(QLatin1String("break"), i18n("Break"), parent)
{
}

SieveAction* SieveActionBreak::newAction()
{
    return new SieveActionBreak;
}

QWidget *SieveActionBreak::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QLabel *lab = new QLabel(i18n("Name (optional):"));
    lay->addWidget(lab);

    KLineEdit *subject = new KLineEdit;
    subject->setObjectName(QLatin1String("name"));
    lay->addWidget(subject);
    return w;
}

bool SieveActionBreak::setParamWidgetValue(const QDomElement &element, QWidget *w , QString &error)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("tag")) {
                const QString tagValue = e.text();
                if (tagValue == QLatin1String("name")) {
                    KLineEdit *name = w->findChild<KLineEdit*>(QLatin1String("name"));
                    name->setText(AutoCreateScriptUtil::strValue(e));
                } else {
                    unknowTagValue(tagValue, error);
                    qDebug()<<" SieveActionBreak::setParamWidgetValue unknown tagValue "<<tagValue;
                }
            } else if (tagName == QLatin1String("str")) {
                //Nothing
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else {
                unknownTag(tagName, error);
                qDebug()<<"SieveActionBreak::setParamWidgetValue unknown tag "<<tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveActionBreak::code(QWidget *w) const
{
    const KLineEdit *name = w->findChild<KLineEdit*>(QLatin1String("name"));
    const QString nameStr = name->text();
    if (!nameStr.isEmpty()) {
        return QString::fromLatin1("break :name \"%1\";").arg(nameStr);
    }
    return QLatin1String("break;");
}

QString SieveActionBreak::help() const
{
    return i18n("The break command terminates the closest enclosing loop.");
}

QStringList SieveActionBreak::needRequires(QWidget */*parent*/) const
{
    return QStringList() << QLatin1String("foreverypart");
}

bool SieveActionBreak::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionBreak::serverNeedsCapability() const
{
    return QLatin1String("foreverypart");
}


#include "sieveactionbreak.moc"
