/*
    This file is part of libkdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kapplication.h>
#include <kcmdlineargs.h>

#include "idmapper.h"

int main( int argc, char **argv )
{
  KApplication::disableAutoDcopRegistration();
  KCmdLineArgs::init( argc, argv, "testemail", 0, 0, 0, 0 );
  KApplication app( false, false );

  KPIM::IdMapper mapper( "test.uidmap" ) ;
  mapper.setIdentifier("testidentifier");

  mapper.setRemoteId( "foo", "bar" );
  mapper.setRemoteId( "yes", "klar" );
  mapper.setRemoteId( "no", "nee" );

  qDebug( "full:\n%s", mapper.asString().toLatin1() );

  mapper.save();

  mapper.clear();
  qDebug( "empty:\n%s", mapper.asString().toLatin1() );

  mapper.load();
  qDebug( "full again:\n%s", mapper.asString().toLatin1() );

  mapper.save();

  mapper.clear();
  qDebug( "empty:\n%s", mapper.asString().toLatin1() );

  mapper.load();
  qDebug( "full again:\n%s", mapper.asString().toLatin1() );
  return 0;
}
