/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "fakeengine.h"
#include <libkmobiletools/ifaces/addressbook.h>

FakeEngine::FakeEngine( QObject *parent )
 : EngineXP( parent )
{
}


FakeEngine::~FakeEngine()
{
}

void FakeEngine::initialize( const QString& deviceName )
{
    Q_UNUSED( deviceName )
}

int FakeEngine::signalStrength() const
{
}

int FakeEngine::charge() const
{
}

KMobileTools::Status::PowerSupplyType FakeEngine::powerSupplyType() const
{
}

bool FakeEngine::ringing() const
{
    return false;
}

QString FakeEngine::networkName() const
{
}

QString FakeEngine::manufacturer() const
{
}

KMobileTools::Information::Manufacturer FakeEngine::manufacturerID() const
{
}

QString FakeEngine::model() const
{
}

QString FakeEngine::imei() const
{
}

QString FakeEngine::revision() const
{
}

void FakeEngine::fetchStatusInformation()
{
}

void FakeEngine::fetchInformation()
{
}

K_EXPORT_COMPONENT_FACTORY( libkmobiletools_fake, FakeEngineFactory )

FakeEngineFactory::FakeEngineFactory()
{
}

FakeEngineFactory::~FakeEngineFactory()
{
}

FakeEngine *FakeEngineFactory::createObject(QObject *parent, const char *classname, const QStringList& args)
{
    Q_UNUSED(classname)
    Q_UNUSED(args)
    return new FakeEngine(parent);
}

#include "fakeengine.moc"
