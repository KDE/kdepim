/*
  This file is part of libkldap.

  Copyright (c) 2002-2009 Tobias Koenig <tokoe@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General  Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "kcmldap_p.h"

#include <QtCore/QString>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcomponentdata.h>
#include <kconfigdialogmanager.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdemacros.h>
#include <kdialogbuttonbox.h>
#include <kgenericfactory.h>
#include <khbox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kvbox.h>

#include "ldapclient.h"
#include <kldap/ldapserver.h>

#include "addhostdialog_p.h"

#ifndef Q_OS_WINCE
K_PLUGIN_FACTORY( KCMLdapFactory, registerPlugin<KCMLdap>(); )
K_EXPORT_PLUGIN( KCMLdapFactory( "kcmldap" ) )
#endif

class LDAPItem : public QListWidgetItem
{
  public:
    LDAPItem( QListWidget *parent, const KLDAP::LdapServer &server, bool isActive = false )
      : QListWidgetItem( parent, QListWidgetItem::UserType ),
        mIsActive( isActive )
    {
      setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable );
      setCheckState( isActive ? Qt::Checked : Qt::Unchecked );
      setServer( server );
    }

    void setServer( const KLDAP::LdapServer &server )
    {
      mServer = server;

      setText( mServer.host() );
    }

    const KLDAP::LdapServer &server() const { return mServer; }

    void setIsActive( bool isActive ) { mIsActive = isActive; }
    bool isActive() const { return mIsActive; }

  private:
    KLDAP::LdapServer mServer;
    bool mIsActive;
};

KCMLdap::KCMLdap( QWidget *parent, const QVariantList& )
#ifdef Q_OS_WINCE
  : KCModule( KGlobal::activeComponent(), parent )
#else
  : KCModule( KCMLdapFactory::componentData(), parent )
#endif // Q_OS_WINCE
{
  KAboutData *about = new KAboutData( I18N_NOOP( "kcmldap" ), 0,
                                      ki18n( "LDAP Server Settings" ),
                                      0, KLocalizedString(), KAboutData::License_LGPL,
                                      ki18n( "(c) 2009 - 2010 Tobias Koenig" ) );

  about->addAuthor( ki18n( "Tobias Koenig" ), KLocalizedString(), "tokoe@kde.org" );

  setAboutData( about );

  initGUI();

  connect( mHostListView, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
           this, SLOT(slotSelectionChanged(QListWidgetItem*)) );
  connect( mHostListView, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
           this, SLOT(slotEditHost()) );
  connect( mHostListView, SIGNAL(itemClicked(QListWidgetItem*)),
           this, SLOT(slotItemClicked(QListWidgetItem*)) );

  connect( mUpButton, SIGNAL(clicked()), this, SLOT(slotMoveUp()) );
  connect( mDownButton, SIGNAL(clicked()), this, SLOT(slotMoveDown()) );
}

KCMLdap::~KCMLdap()
{
}

void KCMLdap::slotSelectionChanged( QListWidgetItem *item )
{
  bool state = ( item != 0 );
  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );
  mDownButton->setEnabled( item && (mHostListView->row( item ) != (mHostListView->count() - 1)) );
  mUpButton->setEnabled( item && (mHostListView->row( item ) != 0) );
}

void KCMLdap::slotItemClicked( QListWidgetItem *item )
{
  LDAPItem *ldapItem = dynamic_cast<LDAPItem*>( item );
  if ( !ldapItem ) {
    return;
  }

  if ( (ldapItem->checkState() == Qt::Checked) != ldapItem->isActive() ) {
    emit changed( true );
    ldapItem->setIsActive( ldapItem->checkState() == Qt::Checked );
  }
}

void KCMLdap::slotAddHost()
{
  KLDAP::LdapServer server;
  AddHostDialog dlg( &server, dialogParent() );

  if ( dlg.exec() && !server.host().isEmpty() ) { //krazy:exclude=crashy
    new LDAPItem( mHostListView, server );

    emit changed( true );
  }
}

void KCMLdap::slotEditHost()
{
  LDAPItem *item = dynamic_cast<LDAPItem*>( mHostListView->currentItem() );
  if ( !item ) {
    return;
  }

  KLDAP::LdapServer server = item->server();
  AddHostDialog dlg( &server, dialogParent() );
  dlg.setCaption( i18n( "Edit Host" ) );

  if ( dlg.exec() && !server.host().isEmpty() ) { //krazy:exclude=crashy
    item->setServer( server );

    emit changed( true );
  }
}

void KCMLdap::slotRemoveHost()
{
  QListWidgetItem *item = mHostListView->takeItem( mHostListView->currentRow() );
  if ( !item ) {
    return;
  }

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
  item->setCheckState( other->isActive() ? Qt::Checked : Qt::Unchecked );
  other->setServer( server );
  other->setIsActive( isActive );
  other->setCheckState( isActive ? Qt::Checked : Qt::Unchecked );
}

void KCMLdap::slotMoveUp()
{
  const QList<QListWidgetItem*> selectedItems = mHostListView->selectedItems();
  if ( selectedItems.count() == 0 ) {
    return;
  }

  LDAPItem *item = static_cast<LDAPItem *>( mHostListView->selectedItems().first() );
  if ( !item ) {
    return;
  }

  LDAPItem *above = static_cast<LDAPItem *>( mHostListView->item( mHostListView->row( item ) - 1 ) );
  if ( !above ) {
    return;
  }

  swapItems( item, above );

  mHostListView->setCurrentItem( above );
  above->setSelected( true );

  emit changed( true );
}

void KCMLdap::slotMoveDown()
{
  const QList<QListWidgetItem*> selectedItems = mHostListView->selectedItems();
  if ( selectedItems.count() == 0 ) {
    return;
  }

  LDAPItem *item = static_cast<LDAPItem *>( mHostListView->selectedItems().first() );
  if ( !item ) {
    return;
  }

  LDAPItem *below = static_cast<LDAPItem *>( mHostListView->item( mHostListView->row( item ) + 1 ) );
  if ( !below ) {
    return;
  }

  swapItems( item, below );

  mHostListView->setCurrentItem( below );
  below->setSelected( true );

  emit changed( true );
}

void KCMLdap::load()
{
  mHostListView->clear();
  KConfig *config = KLDAP::LdapClientSearch::config();
  KConfigGroup group( config, "LDAP" );

  QString host;

  uint count = group.readEntry( "NumSelectedHosts", 0 );
  for ( uint i = 0; i < count; ++i ) {
    KLDAP::LdapServer server;
    KLDAP::LdapClientSearch::readConfig( server, group, i, true );
    LDAPItem *item = new LDAPItem( mHostListView, server, true );
    item->setCheckState( Qt::Checked );
  }

  count = group.readEntry( "NumHosts", 0 );
  for ( uint i = 0; i < count; ++i ) {
    KLDAP::LdapServer server;
    KLDAP::LdapClientSearch::readConfig( server, group, i, false );
    new LDAPItem( mHostListView, server );
  }

  emit changed( false );
}

void KCMLdap::save()
{
  KConfig *config = KLDAP::LdapClientSearch::config();
  config->deleteGroup( "LDAP" );

  KConfigGroup group( config, "LDAP" );

  uint selected = 0;
  uint unselected = 0;
  for ( int i = 0; i < mHostListView->count(); ++i ) {
    LDAPItem *item = dynamic_cast<LDAPItem*>( mHostListView->item( i ) );
    if ( !item ) {
      continue;
    }

    KLDAP::LdapServer server = item->server();
    if ( item->checkState() == Qt::Checked ) {
      KLDAP::LdapClientSearch::writeConfig( server, group, selected, true );
      selected++;
    } else {
      KLDAP::LdapClientSearch::writeConfig( server, group, unselected, false );
      unselected++;
    }
  }

  group.writeEntry( "NumSelectedHosts", selected );
  group.writeEntry( "NumHosts", unselected );
  config->sync();

  emit changed( false );
}

void KCMLdap::defaults()
{
  // add default configuration here
}

void KCMLdap::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( 0 );
  setLayout(layout);

  QGroupBox *groupBox = new QGroupBox( i18n( "LDAP Servers" ), this );
  QVBoxLayout *mainLayout = new QVBoxLayout( groupBox );

  // Contents of the QVGroupBox: label and hbox
  QLabel *label = new QLabel( i18n( "Check all servers that should be used:" ) );
  mainLayout->addWidget( label );

  KHBox *hBox = new KHBox;
  hBox->setSpacing( 6 );
  mainLayout->addWidget(hBox);
  // Contents of the hbox: listview and up/down buttons on the right (vbox)
  mHostListView = new QListWidget( hBox );
  mHostListView->setSortingEnabled( false );

  KVBox *upDownBox = new KVBox( hBox );
  upDownBox->setSpacing( 6 );
  mUpButton = new QToolButton( upDownBox );
  mUpButton->setIcon( KIcon( "go-up" ) );
  mUpButton->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  mUpButton->setEnabled( false ); // b/c no item is selected yet

  mDownButton = new QToolButton( upDownBox );
  mDownButton->setIcon( KIcon( "go-down" ) );
  mDownButton->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  mDownButton->setEnabled( false ); // b/c no item is selected yet

  QWidget *spacer = new QWidget( upDownBox );
  upDownBox->setStretchFactor( spacer, 100 );

  layout->addWidget( groupBox );

  KDialogButtonBox *buttons = new KDialogButtonBox( this );
  buttons->addButton( i18n( "&Add Host..." ),
                      QDialogButtonBox::ActionRole, this, SLOT(slotAddHost()) );
  mEditButton = buttons->addButton( i18n( "&Edit Host..." ),
                                    QDialogButtonBox::ActionRole, this, SLOT(slotEditHost()) );
  mEditButton->setEnabled( false );
  mRemoveButton = buttons->addButton( i18n( "&Remove Host" ),
                                      QDialogButtonBox::ActionRole, this, SLOT(slotRemoveHost()) );
  mRemoveButton->setEnabled( false );
  buttons->layout();

  layout->addWidget( buttons );

  resize( QSize( 460, 300 ).expandedTo( sizeHint() ) );
}

QWidget* KCMLdap::dialogParent()
{
#ifdef Q_WS_MAEMO_5
  return 0;
#else
  return this;
#endif
}

#include "kcmldap_p.moc"
