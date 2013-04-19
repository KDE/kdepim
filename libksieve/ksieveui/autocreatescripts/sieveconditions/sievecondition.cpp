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

#include "sievecondition.h"

#include <QWidget>

using namespace KSieveUi;

SieveCondition::SieveCondition(const QString &name, const QString &label, QObject *parent)
    : QObject(parent), mName(name), mLabel(label)
{
}

SieveCondition::~SieveCondition()
{
}

QString SieveCondition::name() const
{
    return mName;
}

QString SieveCondition::label() const
{
    return mLabel;
}

SieveCondition* SieveCondition::newAction()
{
  return 0;
}

QWidget* SieveCondition::createParamWidget( QWidget *parent ) const
{
  return new QWidget( parent );
}

QString SieveCondition::code(QWidget *parent) const
{
    return QString();
}

QStringList SieveCondition::needRequires() const
{
    return QStringList();
}

bool SieveCondition::needCheckIfServerHasCapability() const
{
    return false;
}

QString SieveCondition::serverNeedsCapability() const
{
    return QString();
}

#include "sievecondition.moc"
