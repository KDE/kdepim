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


#include "sieveconditionihave.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLocale>
#include <KLineEdit>

#include <QWidget>
#include <QHBoxLayout>
#include <QDebug>
#include <QDomNode>

using namespace KSieveUi;
SieveConditionIhave::SieveConditionIhave(QObject *parent)
    : SieveCondition(QLatin1String("ihave"), i18n("IHave"), parent)
{
}

SieveCondition *SieveConditionIhave::newAction()
{
    return new SieveConditionIhave;
}

QWidget *SieveConditionIhave::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    KLineEdit *edit = new KLineEdit;
    edit->setClickMessage(i18n("Use \",\" to separate capabilities"));
    edit->setClearButtonShown(true);
    lay->addWidget(edit);
    edit->setObjectName(QLatin1String("edit"));

    return w;
}

QString SieveConditionIhave::code(QWidget *w) const
{
    const KLineEdit *edit = w->findChild<KLineEdit*>( QLatin1String("edit"));
    const QString editValue = edit->text();
    return QString::fromLatin1("ihave %1").arg(AutoCreateScriptUtil::createList(editValue, QLatin1Char(',')));
}

QStringList SieveConditionIhave::needRequires(QWidget *) const
{
    return QStringList() << QLatin1String("ihave");
}

bool SieveConditionIhave::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionIhave::serverNeedsCapability() const
{
    return QLatin1String("ihave");
}

QString SieveConditionIhave::help() const
{
    return i18n("The \"ihave\" test provides a means for Sieve scripts to test for the existence of a given extension prior to actually using it.");
}

void SieveConditionIhave::setParamWidgetValue(const QDomElement &element, QWidget *w, bool notCondition )
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                const QString tagValue = e.text();
                KLineEdit *edit = w->findChild<KLineEdit*>( QLatin1String("edit"));
                edit->setText(tagValue);
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else {
                qDebug()<<" SieveConditionIhave::setParamWidgetValue unknown tagName "<<tagName;
            }
        }
        node = node.nextSibling();
    }
}

#include "sieveconditionihave.moc"
