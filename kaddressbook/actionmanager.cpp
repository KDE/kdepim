/*
  This file is part of KAddressBook.
  Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include <qapplication.h>
#include <qclipboard.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>

#include "actionmanager.h"
#include "kaddressbook.h"
#include "undo.h"
#include "viewmanager.h"

ActionManager::ActionManager( KXMLGUIClient *client, KAddressBook *widget,
                              bool readWrite, QObject *parent )
  : QObject(parent), mGUIClient( client ), mWidget( widget ), mReadWrite( readWrite )
{
  mACollection = mGUIClient->actionCollection();
  mIsPart = !parent->inherits( "KAddressBookMain" );

  connect( mWidget, SIGNAL( addresseeSelected( bool ) ),
           SLOT( addresseeSelected( bool ) ) );
  connect( mWidget, SIGNAL( modified( bool ) ),
           SLOT( modified( bool ) ) );

  mViewManager = mWidget->viewManager();
  connect( mViewManager, SIGNAL( viewConfigChanged( const QString& ) ),
           SLOT( viewConfigChanged( const QString& ) ) );

  connect( QApplication::clipboard(), SIGNAL( dataChanged() ),
           SLOT( clipboardDataChanged() ) );

  initReadOnlyActions();

  if ( mReadWrite )
    initReadWriteActions();

  KConfig *config = ViewManager::config();
  KConfigGroupSaver viewSaver( config, "Views" );
  mActiveViewName = config->readEntry( "Active" );
  KConfigGroupSaver mwSaver( config, "MainWindow" );
  mActionJumpBar->setChecked( config->readBoolEntry( "JumpBar", false ) );
  mActionExtensions->setCurrentItem( config->readNumEntry( "Extensions", 0 ) );
  mActionDetails->setChecked( config->readBoolEntry( "Details", true ) );

  // set the defaults
  addresseeSelected( false );
  modified( false );
  quickToolsAction();

  connect( UndoStack::instance(), SIGNAL( changed() ), SLOT( updateEditMenu() ) );
  connect( RedoStack::instance(), SIGNAL( changed() ), SLOT( updateEditMenu() ) );
}

ActionManager::~ActionManager()
{
  KConfig *config = ViewManager::config();

  KConfigGroupSaver viewSaver( config, "Views" );
  config->writeEntry( "Active", mActiveViewName );

  KConfigGroupSaver mwSaver( config, "MainWindow" );
  config->writeEntry( "JumpBar", mActionJumpBar->isChecked() );
  config->writeEntry( "Extensions", mActionExtensions->currentItem() );
  config->writeEntry( "Details", mActionDetails->isChecked() );

  config->sync();
}

void ActionManager::setReadWrite( bool rw )
{
  if ( rw == mReadWrite )
    return;

  mReadWrite = rw;
  if ( mReadWrite )
    initReadWriteActions();
  else
    destroyReadWriteActions();
}

void ActionManager::clipboardDataChanged()
{
  if ( mReadWrite )
    mActionPaste->setEnabled( !QApplication::clipboard()->text().isEmpty() );
}

void ActionManager::initReadOnlyActions()
{
  // file menu
  mActionSave = new KAction( i18n( "&Save" ), "filesave", CTRL+Key_S, mWidget,
                             SLOT( save() ), mACollection, "file_sync" );

  new KAction( i18n( "&New Contact..." ), "filenew", CTRL+Key_N, mWidget,
               SLOT( newAddressee() ), mACollection, "file_new_contact" );

  if ( mIsPart )
    mActionMail = new KAction( i18n( "&Mail" ), "mail_generic", 0, mViewManager,
                               SLOT( sendMail() ), mACollection, "kaddressbook_mail" );
  else
    mActionMail = KStdAction::mail( mViewManager, SLOT( sendMail() ), mACollection );

  mActionEditAddressee = new KAction( i18n( "&Edit Contact..." ), "edit", 0,
                                      mWidget, SLOT( editAddressee() ),
                                      mACollection, "file_properties" );

  if ( mIsPart )
    new KAction( i18n( "&Print" ), "fileprint", CTRL + Key_P, mWidget,
                 SLOT( print() ), mACollection, "kaddressbook_print" );
  else
    KStdAction::print( mWidget, SLOT( print() ), mACollection );

  new KAction( i18n( "Import &KDE 2 Address Book..." ), 0, mWidget,
               SLOT( importKDE2() ), mACollection, "file_import_kde2" );

  new KAction( i18n("Import vCard..." ), 0, mWidget, SLOT( importVCardSimple() ),
               mACollection, "file_import_vcard" );

  new KAction( i18n( "&Import List..." ), 0, mWidget, SLOT( importCSV() ),
               mACollection, "file_import_csv" );

  new KAction( i18n( "&Export List..." ), 0, mWidget, SLOT( exportCSV() ),
               mACollection, "file_export_csv" );

  new KAction( i18n( "&Export vCard 3.0..." ), 0, mWidget,
               SLOT( exportVCard30() ), mACollection, "file_export_vcard30" );

  // edit menu
  if ( mIsPart )
    mActionCopy = new KAction( i18n( "&Copy" ), "editcopy", CTRL + Key_C, mViewManager,
                               SLOT( copy() ), mACollection, "kaddressbook_copy" );
  else
    mActionCopy = KStdAction::copy( mViewManager, SLOT( copy() ), mACollection );

  if ( mIsPart )
    new KAction( i18n( "Select &All" ), CTRL + Key_A, mViewManager,
                 SLOT( setSelected() ), mACollection, "kaddressbook_select_all" );
  else
    KStdAction::selectAll( mViewManager, SLOT( setSelected() ), mACollection );

  mActionDelete = new KAction( i18n( "&Delete Contact" ), "editdelete",
                               Key_Delete, mViewManager, SLOT( deleteAddressees() ),
                               mACollection, "edit_delete" );

  if ( mIsPart )
    mActionUndo = new KAction( i18n( "&Undo" ), "undo", CTRL + Key_Z, mWidget,
                               SLOT( undo() ), mACollection, "kaddressbook_undo" );
  else
    mActionUndo = KStdAction::undo( mWidget, SLOT( undo() ), mACollection );
  mActionUndo->setEnabled( false );

  if ( mIsPart )
    mActionRedo = new KAction( i18n( "Re&do" ), "redo", CTRL + SHIFT + Key_Z, mWidget,
                               SLOT( redo() ), mACollection, "kaddressbook_redo" );
  else
    mActionRedo = KStdAction::redo( mWidget, SLOT( redo() ), mACollection );
  mActionRedo->setEnabled( false );

  // view menu
  mActionViewList = new KSelectAction( i18n( "Select View" ), 0, 0, this,
                                       SLOT( slotViewSelected() ), mACollection,
                                       "select_view" );
#if KDE_VERSION >= 309
  mActionViewList->setMenuAccelsEnabled( false );
#endif

  new KAction( i18n( "Modify View..." ), "configure", 0, mViewManager,
               SLOT( modifyView() ), mACollection, "view_modify" );

  new KAction( i18n( "Add View..." ), "window_new", 0, mViewManager,
               SLOT( addView() ), mACollection, "view_add" );

  mActionDeleteView = new KAction( i18n( "Delete View" ), "view_remove", 0,
                                   mViewManager, SLOT( deleteView() ),
                                   mACollection, "view_delete" );

  new KAction( i18n( "Refresh View" ), "reload", 0, mViewManager,
               SLOT( refresh() ), mACollection, "view_refresh" );

  // only enable LDAP lookup if we can handle the protocol
  if ( KProtocolInfo::isKnownProtocol( KURL( "ldap://localhost" ) ) ) {
    new KAction( i18n( "&Lookup Addresses in Directory" ), "find", 0,
                 mWidget, SLOT( slotOpenLDAPDialog() ), mACollection, "ldap_lookup" );
  }

  // settings menu
  mActionExtensions = new KSelectAction( i18n( "Show Extension Bar" ), 0,
                                       mACollection, "options_show_extensions" );
  reloadExtensionNames();
  connect( mActionExtensions, SIGNAL( activated( int ) ),
           mViewManager, SLOT( setActiveExtension( int ) ) );
  connect( mViewManager, SIGNAL( extensionsReloaded() ),
           SLOT( reloadExtensionNames() ) );

  mActionJumpBar = new KToggleAction( i18n( "Show Jump Bar" ), "next", 0,
                                      this, SLOT( quickToolsAction() ),
                                      mACollection, "options_show_jump_bar" );

  mActionDetails = new KToggleAction( i18n( "Show Details" ), 0, 0, this,
                                      SLOT( quickToolsAction() ), mACollection,
                                      "options_show_details" );

  new KAction( i18n( "Edit &Filters..." ), "filter", 0, mWidget,
               SLOT( configureFilters() ), mACollection, "options_edit_filters" );

  mActionSelectFilter = new KSelectAction( i18n( "Select Filter" ), 0, 0, this,
                                           SLOT( slotFilterActivated() ),
                                           mACollection, "select_filter" );
#if KDE_VERSION >= 309
  mActionSelectFilter->setMenuAccelsEnabled( false );
#endif

  connect( this, SIGNAL( filterActivated( int ) ),
           mViewManager, SLOT( filterActivated( int ) ) );
  connect( mViewManager, SIGNAL( filtersEdited() ),
           SLOT( filtersEdited() ) );
  connect( mViewManager, SIGNAL( currentFilterChanged( const QString& ) ),
           SLOT( currentFilterChanged( const QString& ) ) );

  if ( mIsPart ) {
    new KAction( i18n( "&Configure KAddressBook..." ), "configure", 0, mWidget,
                 SLOT( configure() ), mACollection, "kaddressbook_configure" );
    new KAction( i18n( "Configure S&hortcuts..." ), "configure_shortcuts", 0,
                 this, SLOT( keyBindings() ), mACollection, "kaddressbook_configure_shortcuts" );
  } else {
    KStdAction::preferences( mWidget, SLOT( configure() ), mACollection );
    KStdAction::keyBindings( this, SLOT( keyBindings() ), mACollection );
  }
}

void ActionManager::initReadWriteActions()
{
  // edit menu
  if ( mIsPart ) {
    mActionCut = new KAction( i18n( "Cu&t" ), "editcut", CTRL + Key_X, mViewManager,
                              SLOT( cut() ), mACollection, "kaddressbook_cut" );
    mActionPaste = new KAction( i18n( "&Paste" ), "editpaste", CTRL + Key_V, mViewManager,
                                SLOT( paste() ), mACollection, "kaddressbook_paste" );
  } else {
    mActionCut = KStdAction::cut( mViewManager, SLOT( cut() ), mACollection );
    mActionPaste = KStdAction::paste( mViewManager, SLOT( paste() ), mACollection );
  }

  clipboardDataChanged();
}

void ActionManager::destroyReadWriteActions()
{
  delete mActionCut;
  delete mActionPaste;
}

void ActionManager::updateEditMenu()
{
  UndoStack *undo = UndoStack::instance();
  RedoStack *redo = RedoStack::instance();

  if ( undo->isEmpty() )
    mActionUndo->setText( i18n( "Undo" ) );
  else
    mActionUndo->setText( i18n( "Undo %1" ).arg( undo->top()->name() ) );

  mActionUndo->setEnabled( !undo->isEmpty() );

  if ( !redo->top() )
    mActionRedo->setText( i18n( "Redo" ) );
  else
    mActionRedo->setText( i18n( "Redo %1" ).arg( redo->top()->name() ) );

  mActionRedo->setEnabled( !redo->isEmpty() );
}

void ActionManager::addresseeSelected( bool selected )
{
  if ( mReadWrite )
    mActionCut->setEnabled( selected );

  mActionCopy->setEnabled( selected );
  mActionDelete->setEnabled( selected );
  mActionEditAddressee->setEnabled( selected );
  mActionMail->setEnabled( selected );
}

void ActionManager::modified(bool mod)
{
  mModified = mod;
  mActionSave->setEnabled( mod );
}

void ActionManager::initActionViewList()
{
  // Create the view actions, and set the active view
  QStringList viewNameList = mViewManager->viewNames();

  // Just incast there is no active view!
  if ( mActiveViewName.isEmpty() ||
       ( viewNameList.contains( mActiveViewName ) == 0 ) )
    mActiveViewName = *( viewNameList ).at( 0 );

  // delete the current ones
  mActionViewList->clear();

  // Now find the active one and raise it to the top
  mActionViewList->setItems( viewNameList );

  QStringList::Iterator it;
  int i;
  for ( it = viewNameList.begin(), i = 0; it != viewNameList.end(); ++it, ++i ) {
    if ( mActiveViewName == (*it) ) {
      mActionViewList->setCurrentItem( i );
      mViewManager->setActiveView( *it );
    }
  }
}

void ActionManager::viewConfigChanged( const QString &newActive )
{
  if ( !newActive.isEmpty() )
    mActiveViewName = newActive;

  // we need to rebuild the actions
  initActionViewList();

  // Only enable delete if there is more than one view
  mActionDeleteView->setEnabled( mViewManager->viewNames().size() > 1 );
}

void ActionManager::slotViewSelected()
{
  mActiveViewName = mActionViewList->currentText();
  mViewManager->setActiveView( mActiveViewName );
}

void ActionManager::quickToolsAction()
{
  mViewManager->setJumpButtonBarVisible( mActionJumpBar->isChecked() );
  mViewManager->setActiveExtension( mActionExtensions->currentItem() );
  mViewManager->setDetailsVisible( mActionDetails->isChecked() );
}

void ActionManager::filtersEdited()
{
  QString current = mActionSelectFilter->currentText();

  QStringList items( mViewManager->filterNames() );
  items.prepend( i18n( "None" ) );

  mActionSelectFilter->setItems( items );
  currentFilterChanged( current );
}

void ActionManager::slotFilterActivated()
{
  emit filterActivated( mActionSelectFilter->currentItem() - 1 );
}

void ActionManager::currentFilterChanged( const QString& name )
{
  int index = 0;

  if ( !name.isEmpty() ) {
    QStringList items = mActionSelectFilter->items();
    index = items.findIndex( name );
  }

  mActionSelectFilter->setCurrentItem( index );
  emit filterActivated( index - 1 );
}

bool ActionManager::isModified() const
{
  return mModified;
}

void ActionManager::keyBindings()
{
  KKeyDialog::configure( mACollection, true );
}

void ActionManager::reloadExtensionNames()
{
  int pos = mActionExtensions->currentItem();
  mActionExtensions->setItems( mViewManager->extensionNames() );
  mActionExtensions->setCurrentItem( pos );
}

#include "actionmanager.moc"
