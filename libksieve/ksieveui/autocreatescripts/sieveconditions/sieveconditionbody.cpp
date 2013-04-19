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

#include "sieveconditionbody.h"

#include <KLocale>

#include <QWidget>
#include <QHBoxLayout>

using namespace KSieveUi;
SieveConditionBody::SieveConditionBody(QObject *parent)
    : SieveCondition(QLatin1String("body"), i18n("Body"), parent)
{
}

SieveCondition *SieveConditionBody::newAction()
{
    return new SieveConditionBody;
}

QWidget *SieveConditionBody::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
#if 0
    QHBoxLayout *lay = new QHBoxLayout;
    w->setLayout(lay);

    QComboBox *combo = new QComboBox;
    combo->setObjectName(QLatin1String("combosize"));
    combo->addItem(i18n("under"), QLatin1String(":under"));
    combo->addItem(i18n("over"), QLatin1String(":over"));
    lay->addWidget(combo);

    QSpinBox *spinbox = new QSpinBox;
    spinbox->setMinimum(1);
    spinbox->setMaximum(9999);
    spinbox->setSuffix(i18n("kb"));
    spinbox->setObjectName(QLatin1String("spinboxsize"));

    lay->addWidget(spinbox);
#endif
    return w;
}

QString SieveConditionBody::code(QWidget *w) const
{
#if 0
    QComboBox *combo = w->findChild<QComboBox*>( QLatin1String("combosize") );
    const QString comparaison = combo->itemData(combo->currentIndex()).toString();
    QSpinBox *spinbox = w->findChild<QSpinBox*>( QLatin1String("spinboxsize") );
    return QString::fromLatin1("size %1 %2K").arg(comparaison).arg(spinbox->value());
#else
    //TODO
    return QString();
#endif
}

#include "sieveconditionbody.moc"
