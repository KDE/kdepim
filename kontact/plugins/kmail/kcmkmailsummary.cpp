/*
    This file is part of Kontact.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqcheckbox.h>
#include <tqlayout.h>

#include <dcopref.h>

#include <kaboutdata.h>
#include <kaccelmanager.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klistview.h>
#include <klocale.h>

#include "kcmkmailsummary.h"

#include <kdepimmacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_kmailsummary( TQWidget *parent, const char * )
  {
    return new KCMKMailSummary( parent, "kcmkmailsummary" );
  }
}

KCMKMailSummary::KCMKMailSummary( TQWidget *parent, const char *name )
  : KCModule( parent, name )
{
  initGUI();

  connect( mFolderView, TQT_SIGNAL( clicked( TQListViewItem* ) ), TQT_SLOT( modified() ) );
  connect( mFullPath, TQT_SIGNAL( toggled( bool ) ), TQT_SLOT( modified() ) );

  KAcceleratorManager::manage( this );

  load();

  KAboutData *about = new KAboutData( I18N_NOOP( "kcmkmailsummary" ),
                                      I18N_NOOP( "Mail Summary Configuration Dialog" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c) 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
  setAboutData( about );
}

void KCMKMailSummary::modified()
{
  emit changed( true );
}

void KCMKMailSummary::initGUI()
{
  TQVBoxLayout *layout = new TQVBoxLayout( this, 0, KDialog::spacingHint() );

  mFolderView = new KListView( this );
  mFolderView->setRootIsDecorated( true );
  mFolderView->setFullWidth( true );

  mFolderView->addColumn( i18n( "Summary" ) );

  mFullPath = new TQCheckBox( i18n( "Show full path for folders" ), this );

  layout->addWidget( mFolderView );
  layout->addWidget( mFullPath );
}

void KCMKMailSummary::initFolders()
{
  DCOPRef kmail( "kmail", "KMailIface" );

  TQStringList folderList;
  kmail.call( "folderList" ).get( folderList );

  mFolderView->clear();
  mFolderMap.clear();

  TQStringList::Iterator it;
  for ( it = folderList.begin(); it != folderList.end(); ++it ) {
    TQString displayName;
    if ( (*it) == "/Local" )
      displayName = i18n( "prefix for local folders", "Local" );
    else {
      DCOPRef folderRef = kmail.call( "getFolder(TQString)", *it );
      folderRef.call( "displayName()" ).get( displayName );
    }
    if ( (*it).contains( '/' ) == 1 ) {
      if ( mFolderMap.find( *it ) == mFolderMap.end() )
        mFolderMap.insert( *it, new TQListViewItem( mFolderView,
                                                   displayName ) );
    } else {
      const int pos = (*it).findRev( '/' );
      const TQString parentFolder = (*it).left( pos );
      mFolderMap.insert( *it,
                         new TQCheckListItem( mFolderMap[ parentFolder ],
                                             displayName,
                                             TQCheckListItem::CheckBox ) );
    }
  }
}

void KCMKMailSummary::loadFolders()
{
  KConfig config( "kcmkmailsummaryrc" );
  config.setGroup( "General" );

  TQStringList folders;
  if ( !config.hasKey( "ActiveFolders" ) )
    folders << "/Local/inbox";
  else
    folders = config.readListEntry( "ActiveFolders" );

  TQMap<TQString, TQListViewItem*>::Iterator it;
  for ( it = mFolderMap.begin(); it != mFolderMap.end(); ++it ) {
    if ( TQCheckListItem *qli = dynamic_cast<TQCheckListItem*>( it.data() ) ) {
      if ( folders.contains( it.key() ) ) {
        qli->setOn( true );
        mFolderView->ensureItemVisible( it.data() );
      } else {
        qli->setOn( false );
      }
    }
  }
  mFullPath->setChecked( config.readBoolEntry( "ShowFullPath", true ) );
}

void KCMKMailSummary::storeFolders()
{
  KConfig config( "kcmkmailsummaryrc" );
  config.setGroup( "General" );

  TQStringList folders;

  TQMap<TQString, TQListViewItem*>::Iterator it;
  for ( it = mFolderMap.begin(); it != mFolderMap.end(); ++it )
    if ( TQCheckListItem *qli = dynamic_cast<TQCheckListItem*>( it.data() ) )
      if ( qli->isOn() )
        folders.append( it.key() );

  config.writeEntry( "ActiveFolders", folders );
  config.writeEntry( "ShowFullPath", mFullPath->isChecked() );

  config.sync();
}

void KCMKMailSummary::load()
{
  initFolders();
  loadFolders();

  emit changed( false );
}

void KCMKMailSummary::save()
{
  storeFolders();

  emit changed( false );
}

void KCMKMailSummary::defaults()
{
  mFullPath->setChecked( true );

  emit changed( true );
}

#include "kcmkmailsummary.moc"
