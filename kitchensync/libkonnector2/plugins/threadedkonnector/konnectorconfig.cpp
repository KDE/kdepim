/*
    This file is part of KitchenSync.

    Copyright (c) 2003 Mathias Froehlich <Mathias.Froehlich@web.de>

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

#include <kdebug.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kconfig.h>
#include <kapabilities.h>
#include <konnectorinfo.h>

#include "clientmanager.h"

#include "threadedkonnector.h"
#include "konnectorconfig.h"

using namespace Threaded;

ThreadedKonnectorConfig::ThreadedKonnectorConfig( QWidget * parent )
  : KRES::ConfigWidget( parent, 0 ) {
}

ThreadedKonnectorConfig::~ThreadedKonnectorConfig() {
}
    
void ThreadedKonnectorConfig::loadSettings( KRES::Resource * ) {
}

void ThreadedKonnectorConfig::saveSettings( KRES::Resource * ) {
}

#include "konnectorconfig.moc"
