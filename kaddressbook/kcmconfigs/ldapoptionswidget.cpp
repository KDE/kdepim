/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#include <tqgroupbox.h>
#include <tqheader.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqtoolbutton.h>
#include <tqstring.h>

#include <kapplication.h>
#include <kbuttonbox.h>
#include <kconfig.h>
#include <klistview.h>
#include <klocale.h>

#include "addhostdialog.h"
#include "ldapoptionswidget.h"
#include <tqvgroupbox.h>
#include <tqhbox.h>
#include <tqvbox.h>
#include <kiconloader.h>

class LDAPItem : public QCheckListItem
{
  public:
    LDAPItem( TQListView *parent, const KPIM::LdapServer &server, bool isActive = false )
      : TQCheckListItem( parent, parent->lastItem(), TQString::null, TQCheckListItem::CheckBox ),
        mIsActive( isActive )
    {
      setServer( server );
    }

    void setServer( const KPIM::LdapServer &server )
    {
      mServer = server;

      setText( 0, mServer.host() );
    }

    const KPIM::LdapServer &server() const { return mServer; }

    void setIsActive( bool isActive ) { mIsActive = isActive; }
    bool isActive() const { return mIsActive; }

  private:
    KPIM::LdapServer mServer;
    bool mIsActive;
};

LDAPOptionsWidget::LDAPOptionsWidget( TQWidget* parent,  const char* name )
  : TQWidget( parent, name )
{
  initGUI();

  mHostListView->setSorting( -1 );
  mHostListView->setAllColumnsShowFocus( true );
  mHostListView->setFullWidth( true );
  mHostListView->addColumn( TQString::null );
  mHostListView->header()->hide();

  connect( mHostListView, TQT_SIGNAL( selectionChanged( TQListViewItem* ) ),
           TQT_SLOT( slotSelectionChanged( TQListViewItem* ) ) );
  connect( mHostListView, TQT_SIGNAL(doubleClicked( TQListViewItem *, const TQPoint &, int )), this, TQT_SLOT(slotEditHost()));
  connect( mHostListView, TQT_SIGNAL( clicked( TQListViewItem* ) ),
           TQT_SLOT( slotItemClicked( TQListViewItem* ) ) );

  connect( mUpButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotMoveUp() ) );
  connect( mDownButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotMoveDown() ) );
}

LDAPOptionsWidget::~LDAPOptionsWidget()
{
}

void LDAPOptionsWidget::slotSelectionChanged( TQListViewItem *item )
{
  bool state = ( item != 0 );
  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );
  mDownButton->setEnabled( item && item->itemBelow() );
  mUpButton->setEnabled( item && item->itemAbove() );
}

void LDAPOptionsWidget::slotItemClicked( TQListViewItem *item )
{
  LDAPItem *ldapItem = dynamic_cast<LDAPItem*>( item );
  if ( !ldapItem )
    return;

  if ( ldapItem->isOn() != ldapItem->isActive() ) {
    emit changed( true );
    ldapItem->setIsActive( ldapItem->isOn() );
  }
}

void LDAPOptionsWidget::slotAddHost()
{
  KPIM::LdapServer server;
  AddHostDialog dlg( &server, this );

  if ( dlg.exec() && !server.host().isEmpty() ) {
    new LDAPItem( mHostListView, server );

    emit changed( true );
  }
}

void LDAPOptionsWidget::slotEditHost()
{
  LDAPItem *item = dynamic_cast<LDAPItem*>( mHostListView->currentItem() );
  if ( !item )
    return;

  KPIM::LdapServer server = item->server();
  AddHostDialog dlg( &server, this );
  dlg.setCaption( i18n( "Edit Host" ) );

  if ( dlg.exec() && !server.host().isEmpty() ) {
    item->setServer( server );

    emit changed( true );
  }
}

void LDAPOptionsWidget::slotRemoveHost()
{
  TQListViewItem *item = mHostListView->currentItem();
  if ( !item )
    return;

  mHostListView->takeItem( item );
  delete item;

  slotSelectionChanged( mHostListView->currentItem() );

  emit changed( true );
}

static void swapItems( LDAPItem *item, LDAPItem *other )
{
  KPIM::LdapServer server = item->server();
  bool isActive = item->isActive();
  item->setServer( other->server() );
  item->setIsActive( other->isActive() );
  item->setOn( other->isActive() );
  other->setServer( server );
  other->setIsActive( isActive );
  other->setOn( isActive );
}

void LDAPOptionsWidget::slotMoveUp()
{
  LDAPItem *item = static_cast<LDAPItem *>( mHostListView->selectedItem() );
  if ( !item ) return;
  LDAPItem *above = static_cast<LDAPItem *>( item->itemAbove() );
  if ( !above ) return;
  swapItems( item, above );
  mHostListView->setCurrentItem( above );
  mHostListView->setSelected( above, true );
  emit changed( true );
}

void LDAPOptionsWidget::slotMoveDown()
{
  LDAPItem *item = static_cast<LDAPItem *>( mHostListView->selectedItem() );
  if ( !item ) return;
  LDAPItem *below = static_cast<LDAPItem *>( item->itemBelow() );
  if ( !below ) return;
  swapItems( item, below );
  mHostListView->setCurrentItem( below );
  mHostListView->setSelected( below, true );
  emit changed( true );
}

void LDAPOptionsWidget::restoreSettings()
{
  mHostListView->clear();
  KConfig *config = KPIM::LdapSearch::config();
  KConfigGroupSaver saver( config, "LDAP" );

  TQString host;

  uint count = config->readUnsignedNumEntry( "NumSelectedHosts");
  for ( uint i = 0; i < count; ++i ) {
    KPIM::LdapServer server;
    KPIM::LdapSearch::readConfig( server, config, i, true );
    LDAPItem *item = new LDAPItem( mHostListView, server, true );
    item->setOn( true );
  }

  count = config->readUnsignedNumEntry( "NumHosts" );
  for ( uint i = 0; i < count; ++i ) {
    KPIM::LdapServer server;
    KPIM::LdapSearch::readConfig( server, config, i, false );
    new LDAPItem( mHostListView, server );
  }

  emit changed( false );
}

void LDAPOptionsWidget::saveSettings()
{
  KConfig *config = KPIM::LdapSearch::config();
  config->deleteGroup( "LDAP" );

  KConfigGroupSaver saver( config, "LDAP" );

  uint selected = 0; uint unselected = 0;
  TQListViewItemIterator it( mHostListView );
  for ( ; it.current(); ++it ) {
    LDAPItem *item = dynamic_cast<LDAPItem*>( it.current() );
    if ( !item )
      continue;

    KPIM::LdapServer server = item->server();
    if ( item->isOn() ) {
      KPIM::LdapSearch::writeConfig( server, config, selected, true );
      selected++;
    } else {
      KPIM::LdapSearch::writeConfig( server, config, unselected, false );
      unselected++;
    }
  }

  config->writeEntry( "NumSelectedHosts", selected );
  config->writeEntry( "NumHosts", unselected );
  config->sync();

  emit changed( false );
}

void LDAPOptionsWidget::defaults()
{
  // add default configuration here
}

void LDAPOptionsWidget::initGUI()
{
  TQVBoxLayout *layout = new TQVBoxLayout( this, 0, KDialog::spacingHint() );

  TQVGroupBox *groupBox = new TQVGroupBox( i18n( "LDAP Servers" ), this );
  groupBox->setInsideSpacing( KDialog::spacingHint() );
  groupBox->setInsideMargin( KDialog::marginHint() );

  // Contents of the TQVGroupBox: label and hbox
  /*TQLabel *label =*/ new TQLabel( i18n( "Check all servers that should be used:" ), groupBox );

  TQHBox* hBox = new TQHBox( groupBox );
  hBox->setSpacing( 6 );
  // Contents of the hbox: listview and up/down buttons on the right (vbox)
  mHostListView = new KListView( hBox );

  TQVBox* upDownBox = new TQVBox( hBox );
  upDownBox->setSpacing( 6 );
  mUpButton = new TQToolButton( upDownBox, "mUpButton" );
  mUpButton->setIconSet( BarIconSet( "up", KIcon::SizeSmall ) );
  mUpButton->setEnabled( false ); // b/c no item is selected yet

  mDownButton = new TQToolButton( upDownBox, "mDownButton" );
  mDownButton->setIconSet( BarIconSet( "down", KIcon::SizeSmall ) );
  mDownButton->setEnabled( false ); // b/c no item is selected yet

  TQWidget* spacer = new TQWidget( upDownBox );
  upDownBox->setStretchFactor( spacer, 100 );

  layout->addWidget( groupBox );

  KButtonBox *buttons = new KButtonBox( this );
  buttons->addButton( i18n( "&Add Host..." ), this, TQT_SLOT( slotAddHost() ) );
  mEditButton = buttons->addButton( i18n( "&Edit Host..." ), this, TQT_SLOT( slotEditHost() ) );
  mEditButton->setEnabled( false );
  mRemoveButton = buttons->addButton( i18n( "&Remove Host" ), this, TQT_SLOT( slotRemoveHost() ) );
  mRemoveButton->setEnabled( false );
  buttons->layout();

  layout->addWidget( buttons );

  resize( TQSize( 460, 300 ).expandedTo( sizeHint() ) );
}

#include "ldapoptionswidget.moc"
