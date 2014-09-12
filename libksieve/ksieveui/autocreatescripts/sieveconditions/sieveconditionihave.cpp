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

#include "sieveconditionihave.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "editor/sieveeditorutil.h"

#include <KLocalizedString>
#include <QLineEdit>

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

QWidget *SieveConditionIhave::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QLineEdit *edit = new QLineEdit;
    connect(edit, &QLineEdit::textChanged, this, &SieveConditionIhave::valueChanged);
    edit->setPlaceholderText(i18n("Use \",\" to separate capabilities"));
    edit->setClearButtonEnabled(true);
    lay->addWidget(edit);
    edit->setObjectName(QLatin1String("edit"));

    return w;
}

QString SieveConditionIhave::code(QWidget *w) const
{
    const QLineEdit *edit = w->findChild<QLineEdit *>(QLatin1String("edit"));
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

bool SieveConditionIhave::setParamWidgetValue(const QDomElement &element, QWidget *w, bool, QString &error)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                const QString tagValue = e.text();
                QLineEdit *edit = w->findChild<QLineEdit *>(QLatin1String("edit"));
                edit->setText(tagValue);
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qDebug() << " SieveConditionIhave::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveConditionIhave::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

