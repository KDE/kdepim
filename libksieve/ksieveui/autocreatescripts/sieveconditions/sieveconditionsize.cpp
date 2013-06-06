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

#include <KLocale>

#include <QSpinBox>
#include <QHBoxLayout>
#include <QComboBox>

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
    spinbox->setSuffix(QLatin1Char(' ') + i18n("kb"));
    spinbox->setObjectName(QLatin1String("spinboxsize"));

    lay->addWidget(spinbox);
    return w;
}

QString SieveConditionSize::code(QWidget *w) const
{
    const QComboBox *combo = w->findChild<QComboBox*>( QLatin1String("combosize") );
    const QString comparaison = combo->itemData(combo->currentIndex()).toString();
    const QSpinBox *spinbox = w->findChild<QSpinBox*>( QLatin1String("spinboxsize") );
    return QString::fromLatin1("size %1 %2K").arg(comparaison).arg(spinbox->value());
}

QString SieveConditionSize::help() const
{
    return i18n("The \"size\" test deals with the size of a message.  It takes either a tagged argument of \":over\" or \":under\", followed by a number representing the size of the message.");
}

#include "sieveconditionsize.moc"
