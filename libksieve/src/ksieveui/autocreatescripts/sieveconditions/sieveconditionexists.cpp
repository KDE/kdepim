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

#include "sieveconditionexists.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "widgets/selectheadertypecombobox.h"
#include "editor/sieveeditorutil.h"

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QDomNode>
#include "libksieve_debug.h"

using namespace KSieveUi;

SieveConditionExists::SieveConditionExists(QObject *parent)
    : SieveCondition(QStringLiteral("exists"), i18n("Exists"), parent)
{
}

SieveCondition *SieveConditionExists::newAction()
{
    return new SieveConditionExists;
}

QWidget *SieveConditionExists::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QComboBox *combo = new QComboBox;
    combo->setObjectName(QStringLiteral("existscheck"));
    combo->addItem(i18n("exists"), QStringLiteral("exists"));
    combo->addItem(i18n("not exists"), QStringLiteral("not exists"));
    lay->addWidget(combo);
    connect(combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &SieveConditionExists::valueChanged);

    QLabel *lab = new QLabel(i18n("headers:"));
    lay->addWidget(lab);

    SelectHeaderTypeComboBox *value = new SelectHeaderTypeComboBox;
    connect(value, &SelectHeaderTypeComboBox::valueChanged, this, &SieveConditionExists::valueChanged);
    value->setObjectName(QStringLiteral("headervalue"));

    lay->addWidget(value);
    return w;
}

QString SieveConditionExists::code(QWidget *w) const
{
    const QComboBox *combo = w->findChild<QComboBox *>(QStringLiteral("existscheck"));
    const QString comparaison = combo->itemData(combo->currentIndex()).toString();

    const SelectHeaderTypeComboBox *value = w->findChild<SelectHeaderTypeComboBox *>(QStringLiteral("headervalue"));
    return QStringLiteral("%1 %2").arg(comparaison).arg(value->code());
}

QString SieveConditionExists::help() const
{
    return i18n("The \"exists\" test is true if the headers listed in the header-names argument exist within the message.  All of the headers must exist or the test is false.");
}

bool SieveConditionExists::setParamWidgetValue(const QDomElement &element, QWidget *w, bool notCondition, QString &error)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (notCondition) {
                QComboBox *combo = w->findChild<QComboBox *>(QStringLiteral("existscheck"));
                combo->setCurrentIndex(1);
            }
            if (tagName == QLatin1String("str")) {
                SelectHeaderTypeComboBox *value = w->findChild<SelectHeaderTypeComboBox *>(QStringLiteral("headervalue"));
                value->setCode(e.text());
            } else if (tagName == QLatin1String("list")) {
                SelectHeaderTypeComboBox *selectHeaderType = w->findChild<SelectHeaderTypeComboBox *>(QStringLiteral("headervalue"));
                selectHeaderType->setCode(AutoCreateScriptUtil::listValueToStr(e));
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << " SieveConditionExists::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QUrl SieveConditionExists::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

