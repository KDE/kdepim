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

#include "sieveconditionspamtestplus.h"
#include "widgets/selectrelationalmatchtype.h"
#include "widgets/selectcomparatorcombobox.h"

#include <KLocale>

#include <QHBoxLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QDebug>

using namespace KSieveUi;

SieveConditionSpamTestPlus::SieveConditionSpamTestPlus(QObject *parent)
    : SieveCondition(QLatin1String("spamtestplus"), i18n("Spam Test Plus extension"), parent)
{
}

SieveCondition *SieveConditionSpamTestPlus::newAction()
{
    return new SieveConditionSpamTestPlus;
}

QWidget *SieveConditionSpamTestPlus::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QCheckBox *percent = new QCheckBox(i18n("Percent"));
    percent->setObjectName(QLatin1String("percent"));
    lay->addWidget(percent);

    SelectRelationalMatchType *relation = new SelectRelationalMatchType;
    relation->setObjectName(QLatin1String("relation"));
    lay->addWidget(relation);

    SelectComparatorComboBox *comparator = new SelectComparatorComboBox;
    comparator->setObjectName(QLatin1String("comparator"));
    lay->addWidget(comparator);

    QSpinBox *spinbox = new QSpinBox;
    spinbox->setMaximum(100);
    spinbox->setMinimum(0);
    spinbox->setObjectName(QLatin1String("value"));
    lay->addWidget(spinbox);
    return w;
}

QString SieveConditionSpamTestPlus::code(QWidget *w) const
{
    const QCheckBox *checkbox = w->findChild<QCheckBox*>( QLatin1String("percent") );
    const QString percentStr = checkbox->isChecked() ? QLatin1String(":percent") : QString();

    const SelectRelationalMatchType *relation = w->findChild<SelectRelationalMatchType*>( QLatin1String("relation") );
    const QString relationStr = relation->code();

    const SelectComparatorComboBox *comparator = w->findChild<SelectComparatorComboBox*>( QLatin1String("comparator") );
    const QString comparatorStr = comparator->code();

    const QSpinBox *spinbox = w->findChild<QSpinBox*>( QLatin1String("value") );
    const QString value = QString::number(spinbox->value());

    return QString::fromLatin1("spamtest %1 %2 %3 \"%4\"").arg(percentStr).arg(relationStr).arg(comparatorStr).arg(value);
}

bool SieveConditionSpamTestPlus::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionSpamTestPlus::serverNeedsCapability() const
{
    return QLatin1String("spamtestplus");
}

QStringList SieveConditionSpamTestPlus::needRequires(QWidget *w) const
{
    const SelectComparatorComboBox *comparator = w->findChild<SelectComparatorComboBox*>( QLatin1String("comparator") );
    return QStringList() << QLatin1String("spamtestplus") << QLatin1String("relational") << comparator->require();
}

QString SieveConditionSpamTestPlus::help() const
{
    return i18n("Sieve implementations that implement the \"spamtest\" test use an identifier of either \"spamtest\" or \"spamtestplus\" for use with the capability mechanism.");
}

void SieveConditionSpamTestPlus::setParamWidgetValue(const QDomElement &element, QWidget *w )
{
    QCheckBox *checkbox = w->findChild<QCheckBox*>( QLatin1String("percent") );

    SelectRelationalMatchType *relation = w->findChild<SelectRelationalMatchType*>( QLatin1String("relation") );

    SelectComparatorComboBox *comparator = w->findChild<SelectComparatorComboBox*>( QLatin1String("comparator") );

    QSpinBox *spinbox = w->findChild<QSpinBox*>( QLatin1String("value") );

}

#include "sieveconditionspamtestplus.moc"

