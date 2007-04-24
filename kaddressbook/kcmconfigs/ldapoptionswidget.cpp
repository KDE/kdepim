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

#include <q3groupbox.h>
#include <q3header.h>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QToolButton>
#include <QString>
#include <QVBoxLayout>

#include <kapplication.h>
#include <KDialogButtonBox>
#include <kconfig.h>
#include <k3listview.h>
#include <klocale.h>
#include <khbox.h>
#include <kvbox.h>

#include "addhostdialog.h"
#include "ldapoptionswidget.h"
#include <kiconloader.h>
#include <kconfiggroup.h>

class LDAPItem : public Q3CheckListItem
{
  public:
    LDAPItem( Q3ListView *parent, const KLDAP::LdapServer &server, bool isActive = false )
      : Q3CheckListItem( parent, parent->lastItem(), QString(), Q3CheckListItem::CheckBox ),
        mIsActive( isActive )
    {
      setServer( server );
    }

    void setServer( const KLDAP::LdapServer &server )
    {
      mServer = server;

      setText( 0, mServer.host() );
    }

    const KLDAP::LdapServer &server() const { return mServer; }

    void setIsActive( bool isActive ) { mIsActive = isActive; }
    bool isActive() const { return mIsActive; }

  private:
    KLDAP::LdapServer mServer;
    bool mIsActive;
};

LDAPOptionsWidget::LDAPOptionsWidget( QWidget* parent,  const char* name )
  : QWidget( parent )
{
  setObjectName( name );
  initGUI();

  mHostListView->setSorting( -1 );
  mHostListView->setAllColumnsShowFocus( true );
  mHostListView->setFullWidth( true );
  mHostListView->addColumn( QString() );
  mHostListView->header()->hide();

  connect( mHostListView, SIGNAL( selectionChanged( Q3ListViewItem* ) ),
           SLOT( slotSelectionChanged( Q3ListViewItem* ) ) );
  connect( mHostListView, SIGNAL(doubleClicked( Q3ListViewItem *, const QPoint &, int )), this, SLOT(slotEditHost()));
  connect( mHostListView, SIGNAL( clicked( Q3ListViewItem* ) ),
           SLOT( slotItemClicked( Q3ListViewItem* ) ) );

  connect( mUpButton, SIGNAL( clicked() ), this, SLOT( slotMoveUp() ) );
  connect( mDownButton, SIGNAL( clicked() ), this, SLOT( slotMoveDown() ) );
}

LDAPOptionsWidget::~LDAPOptionsWidget()
{
}

void LDAPOptionsWidget::slotSelectionChanged( Q3ListViewItem *item )
{
  bool state = ( item != 0 );
  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );
  mDownButton->setEnabled( item && item->itemBelow() );
  mUpButton->setEnabled( item && item->itemAbove() );
}

void LDAPOptionsWidget::slotItemClicked( Q3ListViewItem *item )
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
  KLDAP::LdapServer server;
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

  KLDAP::LdapServer server = item->server();
  AddHostDialog dlg( &server, this );
  dlg.setCaption( i18n( "Edit Host" ) );

  if ( dlg.exec() && !server.host().isEmpty() ) {
    item->setServer( server );

    emit changed( true );
  }
}

void LDAPOptionsWidget::slotRemoveHost()
{
  Q3ListViewItem *item = mHostListView->currentItem();
  if ( !item )
    return;

  mHostListView->takeItem( item );
  delete item;

  slotSelectionChanged( mHostListView->currentItem() );

  emit changed( true );
}

static void swapItems( LDAPItem *item, LDAPItem *other )
{
  KLDAP::LdapServer server = item->server();
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
  KConfigGroup group( config, "LDAP" );

  QString host;

  uint count = group.readEntry( "NumSelectedHosts", 0);
  for ( uint i = 0; i < count; ++i ) {
    KLDAP::LdapServer server;
    KPIM::LdapSearch::readConfig( server, group, i, true );
    LDAPItem *item = new LDAPItem( mHostListView, server, true );
    item->setOn( true );
  }

  count = group.readEntry( "NumHosts",0 );
  for ( uint i = 0; i < count; ++i ) {
    KLDAP::LdapServer server;
    KPIM::LdapSearch::readConfig( server, group, i, false );
    new LDAPItem( mHostListView, server );
  }

  emit changed( false );
}

void LDAPOptionsWidget::saveSettings()
{
  KConfig *config = KPIM::LdapSearch::config();
  config->deleteGroup( "LDAP" );

  KConfigGroup group( config, "LDAP" );

  uint selected = 0; uint unselected = 0;
  Q3ListViewItemIterator it( mHostListView );
  for ( ; it.current(); ++it ) {
    LDAPItem *item = dynamic_cast<LDAPItem*>( it.current() );
    if ( !item )
      continue;

    KLDAP::LdapServer server = item->server();
    if ( item->isOn() ) {
      KPIM::LdapSearch::writeConfig( server, group, selected, true );
      selected++;
    } else {
      KPIM::LdapSearch::writeConfig( server, group, unselected, false );
      unselected++;
    }
  }

  group.writeEntry( "NumSelectedHosts", selected );
  group.writeEntry( "NumHosts", unselected );
  config->sync();

  emit changed( false );
}

void LDAPOptionsWidget::defaults()
{
  // add default configuration here
}

void LDAPOptionsWidget::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( 0 );

  QGroupBox *groupBox = new QGroupBox( i18n( "LDAP Servers" ), this );
  /*
   * FIXME porting
  groupBox->setInsideSpacing( KDialog::spacingHint() );
  groupBox->setInsideMargin( KDialog::marginHint() );
*/

  // Contents of the QVGroupBox: label and hbox
  /*QLabel *label =*/ new QLabel( i18n( "Check all servers that should be used:" ), groupBox );

  KHBox* hBox = new KHBox( groupBox );
  hBox->setSpacing( 6 );
  // Contents of the hbox: listview and up/down buttons on the right (vbox)
  mHostListView = new K3ListView( hBox );

  KVBox* upDownBox = new KVBox( hBox );
  upDownBox->setSpacing( 6 );
  mUpButton = new QToolButton( upDownBox );
  mUpButton->setObjectName( "mUpButton" );
  mUpButton->setIcon( BarIconSet( "go-up", K3Icon::SizeSmall ) );
  mUpButton->setEnabled( false ); // b/c no item is selected yet

  mDownButton = new QToolButton( upDownBox );
  mDownButton->setObjectName( "mDownButton" );
  mDownButton->setIcon( BarIconSet( "go-down", K3Icon::SizeSmall ) );
  mDownButton->setEnabled( false ); // b/c no item is selected yet

  QWidget* spacer = new QWidget( upDownBox );
  upDownBox->setStretchFactor( spacer, 100 );

  layout->addWidget( groupBox );

  KDialogButtonBox *buttons = new KDialogButtonBox( this );
  buttons->addButton( i18n( "&Add Host..." ), QDialogButtonBox::ActionRole, this, SLOT( slotAddHost() ) );
  mEditButton = buttons->addButton( i18n( "&Edit Host..." ), QDialogButtonBox::ActionRole, this, SLOT( slotEditHost() ) );
  mEditButton->setEnabled( false );
  mRemoveButton = buttons->addButton( i18n( "&Remove Host" ), QDialogButtonBox::ActionRole, this, SLOT( slotRemoveHost() ) );
  mRemoveButton->setEnabled( false );
  buttons->layout();

  layout->addWidget( buttons );

  resize( QSize( 460, 300 ).expandedTo( sizeHint() ) );
}

#include "ldapoptionswidget.moc"
