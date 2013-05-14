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

#include "sieveconditionenvironment.h"

#include <KLocale>
#include <KLineEdit>

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

using namespace KSieveUi;
SieveConditionEnvironment::SieveConditionEnvironment(QObject *parent)
    : SieveCondition(QLatin1String("environment"), i18n("Environment"), parent)
{
}

SieveCondition *SieveConditionEnvironment::newAction()
{
    return new SieveConditionEnvironment;
}

QWidget *SieveConditionEnvironment::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);
    QLabel *lab = new QLabel(i18n("Item:"));
    lay->addWidget(lab);

    KLineEdit *item = new KLineEdit;
    item->setObjectName(QLatin1String("item"));
    lay->addWidget(item);

    lab = new QLabel(i18n("Value:"));
    lay->addWidget(lab);

    KLineEdit *value = new KLineEdit;
    value->setObjectName(QLatin1String("value"));
    lay->addWidget(value);

    return w;
}

QString SieveConditionEnvironment::code(QWidget *w) const
{
    const KLineEdit *item =  w->findChild<KLineEdit*>( QLatin1String("item") );
    const QString itemStr = item->text();

    const KLineEdit *value =  w->findChild<KLineEdit*>( QLatin1String("value") );
    const QString valueStr = value->text();

    return QString::fromLatin1("environment \"%1\" \"%2\"").arg(itemStr).arg(valueStr);
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

#include "sieveconditionenvironment.moc"
