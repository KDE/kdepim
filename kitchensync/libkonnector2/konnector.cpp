/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "konnector.h"

#include "konnectorinfo.h"

using namespace KSync;

Konnector::Konnector( const KConfig *config )
    : KRES::Resource( config )
{
}

Konnector::~Konnector()
{
}

void Konnector::writeConfig( KConfig *config )
{
  KRES::Resource::writeConfig( config );
}

void Konnector::add( const QString& res )
{
    m_resources << res;
}

void Konnector::remove( const QString& res )
{
    m_resources.remove( res );
}

QStringList Konnector::resources() const
{
    return m_resources;
}

bool Konnector::isConnected() const
{
    return info().isConnected();
}

void Konnector::progress( const Progress& prog )
{
    emit sig_progress( this, prog );
}

void Konnector::error( const Error& err )
{
    emit sig_error( this, err );
}

ConfigWidget *Konnector::configWidget( const Kapabilities&, QWidget *,
                                       const char * )
{
    return 0;
}

ConfigWidget *Konnector::configWidget( QWidget *, const char * )
{
    return 0;
}

QStringList Konnector::builtIn() const
{
    return QStringList();
}

#include "konnector.moc"
