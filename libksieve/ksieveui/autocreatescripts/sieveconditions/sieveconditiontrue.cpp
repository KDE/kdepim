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

#include "sieveconditiontrue.h"

#include <KLocale>

#include <QHBoxLayout>
#include <QLabel>

using namespace KSieveUi;

SieveConditionTrue::SieveConditionTrue(QObject *parent)
    : SieveCondition(QLatin1String("true"), i18n("True"), parent)
{
}

SieveCondition *SieveConditionTrue::newAction()
{
    return new SieveConditionTrue;
}

QWidget *SieveConditionTrue::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QLabel *label = new QLabel(i18n("true"));
    lay->addWidget(label);
    return w;
}

QString SieveConditionTrue::code(QWidget *) const
{
    return QLatin1String("true");
}

QString SieveConditionTrue::help() const
{
    return i18n("The \"true\" test always evaluates to true.");
}

bool SieveConditionTrue::setParamWidgetValue(const QDomElement &, QWidget *, bool, QString &error)
{
    //Nothing
    return true;
}

#include "sieveconditiontrue.moc"
