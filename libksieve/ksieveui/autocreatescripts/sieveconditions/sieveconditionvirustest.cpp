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

#include "sieveconditionvirustest.h"

#include "widgets/selectrelationalmatchtype.h"
#include "widgets/selectcomparatorcombobox.h"

#include <KLocale>

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDebug>

using namespace KSieveUi;

SieveConditionVirusTest::SieveConditionVirusTest(QObject *parent)
    : SieveCondition(QLatin1String("virustest"), i18n("Virus Test"), parent)
{
}

SieveCondition *SieveConditionVirusTest::newAction()
{
    return new SieveConditionVirusTest;
}

QWidget *SieveConditionVirusTest::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    SelectRelationalMatchType *relation = new SelectRelationalMatchType;
    relation->setObjectName(QLatin1String("relation"));
    lay->addWidget(relation);

    SelectComparatorComboBox *comparator = new SelectComparatorComboBox;
    comparator->setObjectName(QLatin1String("comparator"));
    lay->addWidget(comparator);

    QSpinBox *spinbox = new QSpinBox;
    spinbox->setMaximum(5);
    spinbox->setMinimum(0);
    spinbox->setObjectName(QLatin1String("value"));
    lay->addWidget(spinbox);
    return w;
}

QString SieveConditionVirusTest::code(QWidget *w) const
{
    const SelectRelationalMatchType *relation = w->findChild<SelectRelationalMatchType*>( QLatin1String("relation") );
    const QString relationStr = relation->code();

    const SelectComparatorComboBox *comparator = w->findChild<SelectComparatorComboBox*>( QLatin1String("comparator") );
    const QString comparatorStr = comparator->code();

    const QSpinBox *spinbox = w->findChild<QSpinBox*>( QLatin1String("value") );
    const QString value = QString::number(spinbox->value());

    return QString::fromLatin1("virustest %1 %2 \"%3\"").arg(relationStr).arg(comparatorStr).arg(value);
}

bool SieveConditionVirusTest::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionVirusTest::serverNeedsCapability() const
{
    return QLatin1String("virustest");
}

QStringList SieveConditionVirusTest::needRequires(QWidget *w) const
{
    const SelectComparatorComboBox *comparator = w->findChild<SelectComparatorComboBox*>( QLatin1String("comparator") );
    return QStringList() << QLatin1String("spamtest") << QLatin1String("relational") << comparator->require();
}

QString SieveConditionVirusTest::help() const
{
    return i18n("Sieve implementations that implement the \"virustest\" test have an identifier of \"virustest\" for use with the capability mechanism.");
}

void SieveConditionVirusTest::setParamWidgetValue(const QDomElement &element, QWidget *parent ) const
{

}

#include "sieveconditionvirustest.moc"

