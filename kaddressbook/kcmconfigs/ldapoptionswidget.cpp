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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qgroupbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qstring.h>

#include <kabc/addresslineedit.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <kconfig.h>
#include <klistview.h>
#include <klocale.h>

#include "addhostdialog.h"
#include "ldapoptionswidget.h"
#include <qvgroupbox.h>
#include <qhbox.h>
#include <qvbox.h>
#include <kiconloader.h>

class LDAPServer
{
  public:
    LDAPServer() : mPort( 389 ) {}
    LDAPServer( const QString &host, int port, const QString &baseDN,
                const QString &bindDN, const QString &pwdBindDN )
      : mHost( host ), mPort( port ), mBaseDN( baseDN ), mBindDN( bindDN ),
        mPwdBindDN( pwdBindDN )
    { }

    QString host() const { return mHost; }
    int port() const { return mPort; }
    QString baseDN() const { return mBaseDN; }
    QString bindDN() const { return mBindDN; }
    QString pwdBindDN() const { return mPwdBindDN; }

    void setHost( const QString &host ) { mHost = host; }
    void setPort( int port ) { mPort = port; }
    void setBaseDN( const QString &baseDN ) {  mBaseDN = baseDN; }
    void setBindDN( const QString &bindDN ) {  mBindDN = bindDN; }
    void setPwdBindDN( const QString &pwdBindDN ) {  mPwdBindDN = pwdBindDN; }

  private:
    QString mHost;
    int mPort;
    QString mBaseDN;
    QString mBindDN;
    QString mPwdBindDN;
};

class LDAPItem : public QCheckListItem
{
  public:
    LDAPItem( QListView *parent, const LDAPServer &server, bool isActive = false )
      : QCheckListItem( parent, parent->lastItem(), QString::null, QCheckListItem::CheckBox ),
        mIsActive( isActive )
    {
      setServer( server );
    }

    void setServer( const LDAPServer &server )
    {
      mServer = server;

      setText( 0, mServer.host() );
    }

    LDAPServer server() const { return mServer; }

    void setIsActive( bool isActive ) { mIsActive = isActive; }
    bool isActive() const { return mIsActive; }

  private:
    LDAPServer mServer;
    bool mIsActive;
};

LDAPOptionsWidget::LDAPOptionsWidget( QWidget* parent,  const char* name )
  : QWidget( parent, name )
{
  initGUI();

  mHostListView->setSorting( -1 );
  mHostListView->addColumn( QString::null );
  mHostListView->header()->hide();

  connect( mHostListView, SIGNAL( selectionChanged( QListViewItem* ) ),
           SLOT( slotSelectionChanged( QListViewItem* ) ) );
  connect( mHostListView, SIGNAL(doubleClicked( QListViewItem *, const QPoint &, int )), this, SLOT(slotEditHost()));
  connect( mHostListView, SIGNAL( clicked( QListViewItem* ) ),
           SLOT( slotItemClicked( QListViewItem* ) ) );

  connect( mUpButton, SIGNAL( clicked() ), this, SLOT( slotMoveUp() ) );
  connect( mDownButton, SIGNAL( clicked() ), this, SLOT( slotMoveDown() ) );
}

LDAPOptionsWidget::~LDAPOptionsWidget()
{
}

void LDAPOptionsWidget::slotSelectionChanged( QListViewItem *item )
{
  bool state = ( item != 0 );
  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );
  mDownButton->setEnabled( item && item->itemBelow() );
  mUpButton->setEnabled( item && item->itemAbove() );
}

void LDAPOptionsWidget::slotItemClicked( QListViewItem *item )
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
  AddHostDialog dlg( this );

  if ( dlg.exec() && !dlg.host().isEmpty() ) {
    LDAPServer server( dlg.host(), dlg.port(), dlg.baseDN(),
                       dlg.bindDN(), dlg.pwdBindDN() );
    new LDAPItem( mHostListView, server );

    emit changed( true );
  }
}

void LDAPOptionsWidget::slotEditHost()
{
  LDAPItem *item = dynamic_cast<LDAPItem*>( mHostListView->currentItem() );
  if ( !item )
    return;

  AddHostDialog dlg( this );
  dlg.setCaption( i18n( "Edit Host" ) );

  dlg.setHost( item->server().host() );
  dlg.setPort( item->server().port() );
  dlg.setBaseDN( item->server().baseDN() );
  dlg.setBindDN( item->server().bindDN() );
  dlg.setPwdBindDN( item->server().pwdBindDN() );

  if ( dlg.exec() && !dlg.host().isEmpty() ) {
    LDAPServer server( dlg.host(), dlg.port(), dlg.baseDN(),
                       dlg.bindDN(), dlg.pwdBindDN() );
    item->setServer( server );

    emit changed( true );
  }
}

void LDAPOptionsWidget::slotRemoveHost()
{
  QListViewItem *item = mHostListView->currentItem();
  if ( !item )
    return;

  mHostListView->takeItem( item );
  delete item;

  slotSelectionChanged( mHostListView->currentItem() );

  emit changed( true );
}

static void swapItems( LDAPItem *item, LDAPItem *other )
{
  LDAPServer server = item->server();
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
  KConfig *config = KABC::AddressLineEdit::config();
  KConfigGroupSaver saver( config, "LDAP" );

  QString host;

  uint count = config->readUnsignedNumEntry( "NumSelectedHosts");
  for ( uint i = 0; i < count; ++i ) {
    LDAPServer server;
    server.setHost( config->readEntry( QString( "SelectedHost%1").arg( i ) ) );
    server.setPort( config->readUnsignedNumEntry( QString( "SelectedPort%1" ).arg( i ) ) );
    server.setBaseDN( config->readEntry( QString( "SelectedBase%1" ).arg( i ) ) );
    server.setBindDN( config->readEntry( QString( "SelectedBind%1" ).arg( i ) ) );
    server.setPwdBindDN( config->readEntry( QString( "SelectedPwdBind%1" ).arg( i ) ) );

    LDAPItem *item = new LDAPItem( mHostListView, server, true );
    item->setOn( true );
  }

  count = config->readUnsignedNumEntry( "NumHosts" );
  for ( uint i = 0; i < count; ++i ) {
    LDAPServer server;
    server.setHost( config->readEntry( QString( "Host%1" ).arg( i ) ) );
    server.setPort( config->readUnsignedNumEntry( QString( "Port%1" ).arg( i ) ) );
    server.setBaseDN( config->readEntry( QString( "Base%1" ).arg( i ) ) );
    server.setBindDN( config->readEntry( QString( "Bind%1" ).arg( i ) ) );
    server.setPwdBindDN( config->readEntry( QString( "PwdBind%1" ).arg( i ) ) );

    new LDAPItem( mHostListView, server );
  }

  emit changed( false );
}

void LDAPOptionsWidget::saveSettings()
{
  KConfig *config = KABC::AddressLineEdit::config();
  config->deleteGroup( "LDAP" );

  KConfigGroupSaver saver( config, "LDAP" );

  uint selected = 0; uint unselected = 0;
  QListViewItemIterator it( mHostListView );
  for ( ; it.current(); ++it ) {
    LDAPItem *item = dynamic_cast<LDAPItem*>( it.current() );
    if ( !item )
      continue;

    LDAPServer server = item->server();
    if ( item->isOn() ) {
      config->writeEntry( QString( "SelectedHost%1" ).arg( selected ), server.host() );
      config->writeEntry( QString( "SelectedPort%1" ).arg( selected ), server.port() );
      config->writeEntry( QString( "SelectedBase%1" ).arg( selected ), server.baseDN() );
      config->writeEntry( QString( "SelectedBind%1" ).arg( selected ), server.bindDN() );
      config->writeEntry( QString( "SelectedPwdBind%1" ).arg( selected ), server.pwdBindDN() );
      selected++;
    } else {
      config->writeEntry( QString( "Host%1" ).arg( unselected ), server.host() );
      config->writeEntry( QString( "Port%1" ).arg( unselected ), server.port() );
      config->writeEntry( QString( "Base%1" ).arg( unselected ), server.baseDN() );
      config->writeEntry( QString( "Bind%1" ).arg( unselected ), server.bindDN() );
      config->writeEntry( QString( "PwdBind%1" ).arg( unselected ), server.pwdBindDN() );
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
  QVBoxLayout *layout = new QVBoxLayout( this, 0, KDialog::spacingHint() );

  QVGroupBox *groupBox = new QVGroupBox( i18n( "LDAP Servers" ), this );
  groupBox->setInsideSpacing( KDialog::spacingHint() );
  groupBox->setInsideMargin( KDialog::marginHint() );

  // Contents of the QVGroupBox: label and hbox
  /*QLabel *label =*/ new QLabel( i18n( "Check all servers that should be used:" ), groupBox );

  QHBox* hBox = new QHBox( groupBox );
  // Contents of the hbox: listview and up/down buttons on the right (vbox)
  mHostListView = new KListView( hBox );

  QVBox* upDownBox = new QVBox( hBox );
  mUpButton = new QToolButton( upDownBox, "mUpButton" );
  mUpButton->setPixmap( BarIcon( "up", KIcon::SizeSmall ) );
  mUpButton->setEnabled( false ); // b/c no item is selected yet

  mDownButton = new QToolButton( upDownBox, "mDownButton" );
  mDownButton->setPixmap( BarIcon( "down", KIcon::SizeSmall ) );
  mDownButton->setEnabled( false ); // b/c no item is selected yet

  QWidget* spacer = new QWidget( upDownBox );
  upDownBox->setStretchFactor( spacer, 100 );

  layout->addWidget( groupBox );

  KButtonBox *buttons = new KButtonBox( this );
  buttons->addButton( i18n( "&Add Host..." ), this, SLOT( slotAddHost() ) );
  mEditButton = buttons->addButton( i18n( "&Edit Host..." ), this, SLOT( slotEditHost() ) );
  mEditButton->setEnabled( false );
  mRemoveButton = buttons->addButton( i18n( "&Remove Host" ), this, SLOT( slotRemoveHost() ) );
  mRemoveButton->setEnabled( false );
  buttons->layout();

  layout->addWidget( buttons );

  resize( QSize( 460, 300 ).expandedTo( sizeHint() ) );
}

#include "ldapoptionswidget.moc"
