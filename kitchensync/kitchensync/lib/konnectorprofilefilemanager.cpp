/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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
#include <kconfig.h>

#include "konnectorprofilefilemanager.h"

using namespace KSync;

KonnectorProfileFileManager::KonnectorProfileFileManager()
{
}

KonnectorProfileFileManager::~KonnectorProfileFileManager()
{
}

KonnectorProfile::ValueList KonnectorProfileFileManager::load()
{
    kdDebug() << "KonnectorProfFileManager::load" << endl;

    QStringList ids;
    QStringList::Iterator it;

    KonnectorProfile::ValueList list;

    KConfig conf("kitchensync_konnectors");

    conf.setGroup("General");
    ids = conf.readListEntry("Ids");
    for ( it = ids.begin(); it != ids.end(); ++it ) {
        kdDebug() << "id " << (*it) << endl;
        conf.setGroup( (*it) );
        KonnectorProfile prof;
        prof.loadFromConfig( &conf );

        /* see if it is valid Transputer had an almost empty config
         * only keys and no values which hit an assert later in the mainwindow
         * ...
         */
        if ( prof.isValid() ) list.append(prof );
    }

    kdDebug() << "KonnectorProfFileManager::load() done" << endl;

    return list;
}

void KonnectorProfileFileManager::save( const KonnectorProfile::ValueList& list)
{
//    kdDebug() << "Saving Profiles " << endl;
    KonnectorProfile::ValueList::ConstIterator it;
    KConfig conf("kitchensync_konnectors");
    QStringList ids;

    for ( it = list.begin();  it != list.end(); ++it ) {
        ids << (*it).uid();
//        kdDebug() << "saving id " << (*it).uid() << " name " << (*it).name() << endl;
        (*it).saveToConfig( &conf );
    }
    conf.setGroup("General");
    conf.writeEntry("Ids", ids );
    conf.sync();
}
