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

#include "sieveconditionsize.h"
#include "widgets/selectsizewidget.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "editor/sieveeditorutil.h"
#include <KLocalizedString>

#include <QHBoxLayout>
#include <QComboBox>
#include <QDomNode>
#include <QDebug>

using namespace KSieveUi;

SieveConditionSize::SieveConditionSize(QObject *parent)
    : SieveCondition(QLatin1String("size"), i18n("Size"), parent)
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
    combo->setObjectName(QLatin1String("combosize"));
    combo->addItem(i18n("under"), QLatin1String(":under"));
    combo->addItem(i18n("over"), QLatin1String(":over"));
    lay->addWidget(combo);
    connect(combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &SieveConditionSize::valueChanged);

    SelectSizeWidget *sizeWidget = new SelectSizeWidget;
    connect(sizeWidget, &SelectSizeWidget::valueChanged, this, &SieveConditionSize::valueChanged);
    sizeWidget->setObjectName(QLatin1String("sizewidget"));
    lay->addWidget(sizeWidget);

    return w;
}

QString SieveConditionSize::code(QWidget *w) const
{
    const QComboBox *combo = w->findChild<QComboBox *>(QLatin1String("combosize"));
    const QString comparaison = combo->itemData(combo->currentIndex()).toString();
    const SelectSizeWidget *sizeWidget = w->findChild<SelectSizeWidget *>(QLatin1String("sizewidget"));
    return QString::fromLatin1("size %1 %2").arg(comparaison).arg(sizeWidget->code());
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
                QComboBox *combo = w->findChild<QComboBox *>(QLatin1String("combosize"));
                const int index = combo->findData(AutoCreateScriptUtil::tagValue(tagValue));
                if (index != -1) {
                    combo->setCurrentIndex(index);
                }
            } else if (tagName == QLatin1String("num")) {
                const qlonglong tagValue = e.text().toLongLong();
                QString numIdentifier;
                if (e.hasAttribute(QLatin1String("quantifier"))) {
                    numIdentifier = e.attribute(QLatin1String("quantifier"));
                }
                SelectSizeWidget *sizeWidget = w->findChild<SelectSizeWidget *>(QLatin1String("sizewidget"));
                sizeWidget->setCode(tagValue, numIdentifier, name(), error);
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qDebug() << " SieveConditionSize::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

QString SieveConditionSize::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}
