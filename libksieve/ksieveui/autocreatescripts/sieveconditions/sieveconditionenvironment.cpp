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

#include "sieveconditionenvironment.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "editor/sieveeditorutil.h"

#include <KLocalizedString>
#include <QLineEdit>

#include <QWidget>
#include <QLabel>
#include <QCompleter>
#include <QDebug>
#include <QDomNode>
#include <QGridLayout>

using namespace KSieveUi;
SieveConditionEnvironment::SieveConditionEnvironment(QObject *parent)
    : SieveCondition(QLatin1String("environment"), i18n("Environment"), parent)
{
}

SieveCondition *SieveConditionEnvironment::newAction()
{
    return new SieveConditionEnvironment;
}

QWidget *SieveConditionEnvironment::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    w->setLayout(grid);
    QLabel *lab = new QLabel(i18n("Item:"));
    grid->addWidget(lab, 0, 0);

    QLineEdit *item = new QLineEdit;
    QStringList itemList;
    itemList << QLatin1String("domain")
             << QLatin1String("host")
             << QLatin1String("location")
             << QLatin1String("name")
             << QLatin1String("phase")
             << QLatin1String("remote-host")
             << QLatin1String("remote-ip")
             << QLatin1String("version");
    QCompleter *completer = new QCompleter(itemList, w);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    item->setCompleter(completer);
    connect(item, &QLineEdit::textChanged, this, &SieveConditionEnvironment::valueChanged);

    item->setObjectName(QLatin1String("item"));
    grid->addWidget(item, 0, 1);

    lab = new QLabel(i18n("Value:"));
    grid->addWidget(lab, 1, 0);

    QLineEdit *value = new QLineEdit;
    connect(value, &QLineEdit::textChanged, this, &SieveConditionEnvironment::valueChanged);
    value->setObjectName(QLatin1String("value"));
    grid->addWidget(value, 1, 1);

    return w;
}

QString SieveConditionEnvironment::code(QWidget *w) const
{
    const QLineEdit *item =  w->findChild<QLineEdit *>(QLatin1String("item"));
    const QString itemStr = item->text();

    const QLineEdit *value =  w->findChild<QLineEdit *>(QLatin1String("value"));
    const QString valueStr = value->text();

    return QStringLiteral("environment \"%1\" \"%2\"").arg(itemStr).arg(valueStr);
}

QStringList SieveConditionEnvironment::needRequires(QWidget *) const
{
    return QStringList() << QLatin1String("environment");
}

bool SieveConditionEnvironment::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionEnvironment::serverNeedsCapability() const
{
    return QLatin1String("environment");
}

QString SieveConditionEnvironment::help() const
{
    return i18n("The environment test retrieves the item of environment information specified by the name string and matches it to the values specified in the key-list argument.");
}

bool SieveConditionEnvironment::setParamWidgetValue(const QDomElement &element, QWidget *w, bool /*notCondition*/ , QString &error)
{
    QDomNode node = element.firstChild();
    int index = 0;
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                if (index == 0) {
                    QLineEdit *item =  w->findChild<QLineEdit *>(QLatin1String("item"));
                    item->setText(AutoCreateScriptUtil::quoteStr(e.text()));
                } else if (index == 1) {
                    QLineEdit *value =  w->findChild<QLineEdit *>(QLatin1String("value"));
                    value->setText(AutoCreateScriptUtil::quoteStr(e.text()));
                } else {
                    tooManyArgument(tagName, index, 2, error);
                    qDebug() << " SieveConditionEnvironment::setParamWidgetValue to many argument " << index;
                }
                ++index;
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qDebug() << " SieveActionSetVariable::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString KSieveUi::SieveConditionEnvironment::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}
