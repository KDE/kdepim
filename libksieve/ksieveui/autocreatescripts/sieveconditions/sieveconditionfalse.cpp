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

#include "sieveconditionfalse.h"

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QLabel>

using namespace KSieveUi;

SieveConditionFalse::SieveConditionFalse(QObject *parent)
    : SieveCondition(QLatin1String("false"), i18n("False"), parent)
{
}

SieveCondition *SieveConditionFalse::newAction()
{
    return new SieveConditionFalse;
}

QWidget *SieveConditionFalse::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QLabel *label = new QLabel(i18n("false"));
    lay->addWidget(label);
    return w;
}

QString SieveConditionFalse::code(QWidget *) const
{
    return QLatin1String("false");
}

QString SieveConditionFalse::help() const
{
    return i18n("The \"false\" test always evaluates to false.");
}

bool SieveConditionFalse::setParamWidgetValue(const QDomElement &, QWidget *, bool, QString &)
{
    //Nothing
    return true;
}

QString SieveConditionFalse::href() const
{
    return QLatin1String("http://tools.ietf.org/html/rfc3028#page-25");
}

