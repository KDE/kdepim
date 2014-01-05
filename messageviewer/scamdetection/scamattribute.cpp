/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "scamattribute.h"
#include <QByteArray>
#include <QIODevice>
#include <QDataStream>

using namespace MessageViewer;

class MessageViewer::ScamAttributePrivate
{
public:
    ScamAttributePrivate()
        : isAScam(false)
    {
    }
    bool isAScam;
};

ScamAttribute::ScamAttribute()
    : d(new ScamAttributePrivate)
{
}

ScamAttribute::~ScamAttribute()
{
    delete d;
}

ScamAttribute *ScamAttribute::clone() const
{
    ScamAttribute *attr = new ScamAttribute();
    attr->setIsAScam(isAScam());
    return attr;
}

QByteArray ScamAttribute::type() const
{
    static const QByteArray sType( "ScamAttribute" );
    return sType;
}

QByteArray ScamAttribute::serialized() const
{
    QByteArray result;
    QDataStream s( &result, QIODevice::WriteOnly );
    s << isAScam();
    return result;
}

void ScamAttribute::deserialize( const QByteArray &data )
{
    QDataStream s( data );
    bool value = false;
    s >> value;
    d->isAScam = value;
}

bool ScamAttribute::isAScam() const
{
    return d->isAScam;
}

void ScamAttribute::setIsAScam(bool b)
{
    d->isAScam = b;
}
