/*
    This file is part of KitchenSync.

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

#include <klocale.h>
#include <kstatusbar.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>

#include "actionmanager.h"

using namespace KSync;

ActionManager::ActionManager( KActionCollection *actionCollection )
  : mActionCollection( actionCollection ), mView( 0 )
{
}

ActionManager::~ActionManager()
{
  writeConfig();
}

void ActionManager::setView( KitchenSync *view )
{
  mView = view;
}

void ActionManager::initActions()
{
  if ( !mView ) {
    kdError() << "Call KSync::ActionManager::setView() before "
              << "KSync::ActionManager::initActions()." << endl;
    return;
  }

  new KAction( i18n("Synchronize" ), "reload", 0, mView, SLOT( slotSync() ),
               mActionCollection, "sync" );

  new KAction( i18n("Configure Profiles..."), "configure", 0,
               mView, SLOT( configureProfiles() ),
               mActionCollection, "config_profile" );

  new KAction( i18n("Configure Current Profile..."), "configure", 0,
               mView, SLOT( configureCurrentProfile() ),
               mActionCollection, "config_current" );

  m_profAct = new KSelectAction( i18n("Profile"), KShortcut(), mView,
                                 SLOT( activateProfile() ),
                                 mActionCollection, "select_prof");

  KStdAction::preferences( mView, SLOT( slotPreferences() ),
                           mActionCollection );
}

int ActionManager::currentProfile()
{
  return m_profAct->currentItem();
}

void ActionManager::setProfiles( const QStringList &profiles )
{
  m_profAct->setItems( profiles );
}

void ActionManager::readConfig()
{
  KConfig *cfg = KGlobal::config();
  cfg->setGroup( "Profiles" );
  int currentProfile = cfg->readNumEntry( "CurrentProfile", 0 );
  m_profAct->setCurrentItem( currentProfile );
}

void ActionManager::writeConfig()
{
  KConfig *cfg = KGlobal::config();
  cfg->setGroup( "Profiles" );
  int currentProfile = m_profAct->currentItem();
  cfg->writeEntry( "CurrentProfile", currentProfile );
  cfg->sync();
}
