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

#include "sieveaction.h"

#include <QWidget>

using namespace KSieveUi;

SieveAction::SieveAction(const QString &name, const QString &label, QObject *parent)
    : QObject(parent), mName(name), mLabel(label)
{
}

SieveAction::~SieveAction()
{
}

QString SieveAction::name() const
{
    return mName;
}

QString SieveAction::label() const
{
    return mLabel;
}

SieveAction* SieveAction::newAction()
{
  return 0;
}

QWidget* SieveAction::createParamWidget( QWidget *parent ) const
{
  return new QWidget( parent );
}

QString SieveAction::code(QWidget *) const
{
    return QString();
}

QStringList SieveAction::needRequires() const
{
    return QStringList();
}

#include "sieveaction.moc"
