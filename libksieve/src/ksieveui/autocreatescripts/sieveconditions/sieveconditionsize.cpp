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

#include "sieveconditionsize.h"
#include "widgets/selectsizewidget.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "editor/sieveeditorutil.h"
#include <KLocalizedString>

#include <QHBoxLayout>
#include <QComboBox>
#include <QDomNode>
#include "libksieve_debug.h"

using namespace KSieveUi;

SieveConditionSize::SieveConditionSize(QObject *parent)
    : SieveCondition(QStringLiteral("size"), i18n("Size"), parent)
{
}

SieveCondition *SieveConditionSize::newAction()
{
    return new SieveConditionSize;
}

QWidget *SieveConditionSize::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QComboBox *combo = new QComboBox;
    combo->setObjectName(QStringLiteral("combosize"));
    combo->addItem(i18n("under"), QStringLiteral(":under"));
    combo->addItem(i18n("over"), QStringLiteral(":over"));
    lay->addWidget(combo);
    connect(combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &SieveConditionSize::valueChanged);

    SelectSizeWidget *sizeWidget = new SelectSizeWidget;
    connect(sizeWidget, &SelectSizeWidget::valueChanged, this, &SieveConditionSize::valueChanged);
    sizeWidget->setObjectName(QStringLiteral("sizewidget"));
    lay->addWidget(sizeWidget);

    return w;
}

QString SieveConditionSize::code(QWidget *w) const
{
    const QComboBox *combo = w->findChild<QComboBox *>(QStringLiteral("combosize"));
    const QString comparaison = combo->itemData(combo->currentIndex()).toString();
    const SelectSizeWidget *sizeWidget = w->findChild<SelectSizeWidget *>(QStringLiteral("sizewidget"));
    return QStringLiteral("size %1 %2").arg(comparaison).arg(sizeWidget->code());
}

QString SieveConditionSize::help() const
{
    return i18n("The \"size\" test deals with the size of a message.  It takes either a tagged argument of \":over\" or \":under\", followed by a number representing the size of the message.");
}

bool SieveConditionSize::setParamWidgetValue(const QDomElement &element, QWidget *w, bool /*notCondition*/, QString &error)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("tag")) {
                const QString tagValue = e.text();
                QComboBox *combo = w->findChild<QComboBox *>(QStringLiteral("combosize"));
                const int index = combo->findData(AutoCreateScriptUtil::tagValue(tagValue));
                if (index != -1) {
                    combo->setCurrentIndex(index);
                }
            } else if (tagName == QLatin1String("num")) {
                const qlonglong tagValue = e.text().toLongLong();
                QString numIdentifier;
                if (e.hasAttribute(QStringLiteral("quantifier"))) {
                    numIdentifier = e.attribute(QStringLiteral("quantifier"));
                }
                SelectSizeWidget *sizeWidget = w->findChild<SelectSizeWidget *>(QStringLiteral("sizewidget"));
                sizeWidget->setCode(tagValue, numIdentifier, name(), error);
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << " SieveConditionSize::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QUrl SieveConditionSize::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}
