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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qgroupbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>

#include <kapplication.h>
#include <kbuttonbox.h>
#include <kconfig.h>
#include <klistview.h>
#include <klocale.h>

#include "addhostdialog.h"
#include "ldapoptionswidget.h"
#include "viewmanager.h"

class LDAPServer
{
  public:
    LDAPServer() : mPort( 389 ) {}
    LDAPServer( const QString &host, int port, const QString &baseDN )
      : mHost( host ), mPort( port ), mBaseDN( baseDN )
    { }

    QString host() const { return mHost; }
    int port() const { return mPort; }
    QString baseDN() const { return mBaseDN; }

    void setHost( const QString &host ) { mHost = host; }
    void setPort( int port ) { mPort = port; }
    void setBaseDN( const QString &baseDN ) {  mBaseDN = baseDN; }

  private:
    QString mHost;
    int mPort;
    QString mBaseDN;
};

class LDAPItem : public QCheckListItem
{
  public:
    LDAPItem( QListView *parent, const LDAPServer &server )
      : QCheckListItem( parent, QString::null, QCheckListItem::CheckBox )
    {
      setServer( server );
    }

    void setServer( const LDAPServer &server )
    {
      mServer = server;

      setText( 0, mServer.host() );
    }

    LDAPServer server() const { return mServer; }

  private:
    LDAPServer mServer;
};

LDAPOptionsWidget::LDAPOptionsWidget( QWidget* parent,  const char* name )
  : QWidget( parent, name )
{
  initGUI();

  mHostListView->addColumn( QString::null );
  mHostListView->header()->hide();

  connect( mHostListView, SIGNAL( selectionChanged( QListViewItem* ) ),
           SLOT( slotSelectionChanged( QListViewItem* ) ) );
  connect( mHostListView, SIGNAL(doubleClicked( QListViewItem *, const QPoint &, int )), this, SLOT(slotEditHost()));
}

LDAPOptionsWidget::~LDAPOptionsWidget()
{
}

void LDAPOptionsWidget::slotSelectionChanged( QListViewItem *item )
{
  bool state = ( item != 0 );

  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );
}

void LDAPOptionsWidget::slotAddHost()
{
  AddHostDialog dlg( this );

  if ( dlg.exec() && !dlg.host().isEmpty() ) {
    LDAPServer server( dlg.host(), dlg.port(), dlg.baseDN() );
    new LDAPItem( mHostListView, server );
  }
}

void LDAPOptionsWidget::slotEditHost()
{
  LDAPItem *item = dynamic_cast<LDAPItem*>( mHostListView->currentItem() );
  if ( !item )
    return;

  AddHostDialog dlg( this );
  dlg.setCaption( i18n("Edit Host") );

  dlg.setHost( item->server().host() );
  dlg.setPort( item->server().port() );
  dlg.setBaseDN( item->server().baseDN() );

  if ( dlg.exec() && !dlg.host().isEmpty() ) {
    LDAPServer server( dlg.host(), dlg.port(), dlg.baseDN() );
    item->setServer( server );
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
}

void LDAPOptionsWidget::restoreSettings()
{
  KConfig *config = ViewManager::config();
  config->setGroup( "LDAP" );

  QString host;

  uint count = config->readUnsignedNumEntry( "NumSelectedHosts");
  for ( uint i = 0; i < count; ++i ) {
    LDAPServer server;
    server.setHost( config->readEntry( QString( "SelectedHost%1").arg( i ) ) );
    server.setPort( config->readUnsignedNumEntry( QString( "SelectedPort%1" ).arg( i ) ) );
    server.setBaseDN( config->readEntry( QString( "SelectedBase%1" ).arg( i ) ) );

    LDAPItem *item = new LDAPItem( mHostListView, server );
    item->setOn( true );
  }

  count = config->readUnsignedNumEntry( "NumHosts" );
  for ( uint i = 0; i < count; ++i ) {
    LDAPServer server;
    server.setHost( config->readEntry( QString( "Host%1" ).arg( i ) ) );
    server.setPort( config->readUnsignedNumEntry( QString( "Port%1" ).arg( i ) ) );
    server.setBaseDN( config->readEntry( QString( "Base%1" ).arg( i ) ) );
    new LDAPItem( mHostListView, server );
  }
}

void LDAPOptionsWidget::saveSettings()
{
  KConfig *config = ViewManager::config();
  config->deleteGroup( "LDAP" );

  config->setGroup("LDAP");

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
      selected++;
    } else {
      config->writeEntry( QString( "Host%1" ).arg( selected ), server.host() );
      config->writeEntry( QString( "Port%1" ).arg( selected ), server.port() );
      config->writeEntry( QString( "Base%1" ).arg( selected ), server.baseDN() );
      unselected++;
    }
  }

  config->writeEntry( "NumSelectedHosts", selected );
  config->writeEntry( "NumHosts", unselected );
  config->sync();
}

void LDAPOptionsWidget::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  QGroupBox *groupBox = new QGroupBox( i18n( "LDAP Servers" ), this );
  groupBox->setColumnLayout( 0, Qt::Vertical );
  groupBox->layout()->setSpacing( KDialog::spacingHint() );
  groupBox->layout()->setMargin( KDialog::marginHint() );

  QVBoxLayout *groupBoxLayout = new QVBoxLayout( groupBox->layout() );
  groupBoxLayout->setAlignment( Qt::AlignTop );

  QLabel *label = new QLabel( i18n( "Check all servers that should be used:" ), groupBox );
  groupBoxLayout->addWidget( label );

  mHostListView = new KListView( groupBox );
  groupBoxLayout->addWidget( mHostListView );

  layout->addWidget( groupBox );
  layout->addStretch();

  KButtonBox *buttons = new KButtonBox( this );
  buttons->addStretch();
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
