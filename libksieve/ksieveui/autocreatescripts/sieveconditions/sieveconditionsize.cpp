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

#include "sieveconditionsize.h"
#include "widgets/selectsizetypecombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLocale>

#include <QSpinBox>
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

QWidget *SieveConditionSize::createParamWidget( QWidget *parent ) const
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

    QSpinBox *spinbox = new QSpinBox;
    spinbox->setMinimum(1);
    spinbox->setMaximum(9999);
    lay->addWidget(spinbox);
    spinbox->setObjectName(QLatin1String("spinboxsize"));

    SelectSizeTypeComboBox *sizeType = new SelectSizeTypeComboBox;
    sizeType->setObjectName(QLatin1String("sizetype"));
    lay->addWidget(sizeType);


    return w;
}

QString SieveConditionSize::code(QWidget *w) const
{
    const QComboBox *combo = w->findChild<QComboBox*>( QLatin1String("combosize") );
    const QString comparaison = combo->itemData(combo->currentIndex()).toString();
    const QSpinBox *spinbox = w->findChild<QSpinBox*>( QLatin1String("spinboxsize") );
    const SelectSizeTypeComboBox *sizeTypeCombo = w->findChild<SelectSizeTypeComboBox*>( QLatin1String("sizetype") );
    const QString type = sizeTypeCombo->code();
    return QString::fromLatin1("size %1 %2%3").arg(comparaison).arg(spinbox->value()).arg(type);
}

QString SieveConditionSize::help() const
{
    return i18n("The \"size\" test deals with the size of a message.  It takes either a tagged argument of \":over\" or \":under\", followed by a number representing the size of the message.");
}

void SieveConditionSize::setParamWidgetValue(const QDomElement &element, QWidget *w )
{
    QComboBox *combo = w->findChild<QComboBox*>( QLatin1String("combosize") );
    QSpinBox *spinbox = w->findChild<QSpinBox*>( QLatin1String("spinboxsize") );
    SelectSizeTypeComboBox *sizeTypeCombo = w->findChild<SelectSizeTypeComboBox*>( QLatin1String("sizetype") );
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("tag")) {
                const QString tagValue = e.text();
                const int index = combo->findData(AutoCreateScriptUtil::tagValue(tagValue));
                if (index != -1) {
                    combo->setCurrentIndex(index);
                }
            } else if (tagName == QLatin1String("num")) {
                const int tagValue = e.text().toInt();
                //TODO fix value
                if (element.hasAttribute(QLatin1String("quantifier"))) {
                    const QString numIdentifier = element.attribute(QLatin1String("quantifier"));
                    sizeTypeCombo->setCode(numIdentifier);
                }
            } else {
                qDebug()<<" SieveConditionSize::setParamWidgetValue unknown tagName "<<tagName;
            }
        }
        node = node.nextSibling();
    }
}

#include "sieveconditionsize.moc"
