/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "kcal_resourcegroupwise.h"

#include "kcal_groupwiseprefs.h"
#include "kcal_resourcegroupwiseconfig.h"

#include "confirmsavedialog.h"

#include "soap/groupwiseserver.h"

#include <qapplication.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qtimer.h>

#include <kabc/locknull.h>
#include <kdebug.h>
#include <klocale.h>
#include <kresources/configwidget.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kurl.h>
#include <libkdepim/kpimprefs.h>

using namespace KCal;

ResourceGroupwise::ResourceGroupwise()
  : ResourceCached( 0 ), mLock( true ), mServer( 0 )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  initGroupwise();
}

ResourceGroupwise::ResourceGroupwise( const KConfig *config )
  : ResourceCached( config ), mLock( true ), mServer( 0 )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  if ( config ) readConfig( config );
  
  initGroupwise();
}

ResourceGroupwise::~ResourceGroupwise()
{
  disableChangeNotification();

  delete mServer;
  mServer = 0;

  delete mPrefs;
  mPrefs = 0;
}

void ResourceGroupwise::init()
{
  mPrefs = new GroupwisePrefs();
  
  setType( "groupwise" );

  enableChangeNotification();
}

void ResourceGroupwise::initGroupwise()
{
  QString url;
  url =  "http://" + mPrefs->host() +":";
  url += QString::number(mPrefs->port()) + "/soap/";
  mServer = new GroupwiseServer( url, mPrefs->user(),
                                 mPrefs->password(), this );

  connect( mServer, SIGNAL( readCalendarFinished() ),
           this, SLOT( loadFinished() ) );
}

GroupwisePrefs *ResourceGroupwise::prefs()
{
  return mPrefs;
}

void ResourceGroupwise::readConfig( const KConfig * )
{
  kdDebug() << "KCal::ResourceGroupwise::readConfig()" << endl;

  mPrefs->readConfig();

  kdDebug() << "HOST: " << mPrefs->host() << endl;
}

void ResourceGroupwise::writeConfig( KConfig *config )
{
  kdDebug() << "KCal::ResourceGroupwise::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  mPrefs->writeConfig();

  ResourceCached::writeConfig( config );
}

bool ResourceGroupwise::doOpen()
{
  return mServer->login();
}

void ResourceGroupwise::doClose()
{
  mServer->logout();
}

bool ResourceGroupwise::doLoad()
{
  kdDebug() << "ResourceGroupwise::load()" << endl;

  if ( !mServer ) {
    kdError() << "KCal::ResourceGrouwise::doLoad(): No Server set." << endl;
    return false;
  }

  mCalendar.close();

  disableChangeNotification();
  loadCache();

  clearChanges();

  mServer->readCalendar( &mCalendar, this );

  return true;
}

void ResourceGroupwise::loadFinished()
{
  saveCache();
  enableChangeNotification();

  emit resourceLoaded( this );
  emit resourceChanged( this );
}

bool ResourceGroupwise::doSave()
{
  saveCache();

  if ( !hasChanges() ) return true;
  
  if ( !confirmSave() ) return false;

  Incidence::List::ConstIterator it;

  Incidence::List added = addedIncidences();
  for( it = added.begin(); it != added.end(); ++it ) {
    if ( mServer->addIncidence( *it ) ) clearChange( *it );
  }
  Incidence::List changed = changedIncidences();
  for( it = changed.begin(); it != changed.end(); ++it ) {
    if ( mServer->changeIncidence( *it ) ) clearChange( *it );
  }
  Incidence::List deleted = deletedIncidences();
  for( it = deleted.begin(); it != deleted.end(); ++it ) {
    if ( mServer->deleteIncidence( *it ) ) clearChange( *it );
  }

  return true;
}

// FIXME: Put this into ResourceCached
bool ResourceGroupwise::confirmSave()
{
  if ( !hasChanges() ) return true;

  ConfirmSaveDialog dlg( resourceName(), 0 );

  dlg.addIncidences( addedIncidences(), i18n("Added") );
  dlg.addIncidences( changedIncidences(), i18n("Changed") );
  dlg.addIncidences( deletedIncidences(), i18n("Deleted") );

  int result = dlg.exec();
  return result == QDialog::Accepted;
}

KABC::Lock *ResourceGroupwise::lock()
{
  return &mLock;
}


#include "kcal_resourcegroupwise.moc"
