/*
  IM address editor widget for KAddressBook

  Copyright (c) 2004 Will Stephenson   <lists@stevello.free-online.co.uk>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include <q3listview.h>
#include <QStringList>
#include <QFont>
#include <QList>

#include <kdialog.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kplugininfo.h>
#include <kpushbutton.h>
#include <kservicetypetrader.h>

#include "imaddresswidget.h"
#include "ui_imeditorbase.h"
#include "imeditorwidget.h"

IMAddressLVI::IMAddressLVI( QTreeWidget *parent, const KPluginInfo &protocol,
                            const QString &address, const IMContext &context )
  : QTreeWidgetItem( parent )
{
  setProtocol( protocol );
  setAddress( address );
  setContext( context );
  mPreferred = false;
}

void IMAddressLVI::setPreferred( bool preferred )
{
  mPreferred = preferred;
  QFont f( font( 0 ) );
  f.setBold( preferred );
  setFont( 0, f );
  setFont( 1, f );
}

bool IMAddressLVI::preferred() const
{
  return mPreferred;
}

void IMAddressLVI::setAddress( const QString &address )
{
  // irc uses 0xE120 to separate the nick and the server group.
  QString serverOrGroup = address.section( QChar( 0xE120 ), 1 );

  if ( serverOrGroup.isEmpty() ) {
    setText( 1, address );
  } else {
    QString nickname = address.section( QChar( 0xE120 ), 0, 0 );
    setText( 1, i18nc( "<nickname> on <server>","%1 on %2" ,
                       nickname, serverOrGroup ) );
  }

  mAddress = address;
}

void IMAddressLVI::setContext( const IMContext &context )
{
  mContext = context;
  // set context
/*  switch ( context )
    {
    case Home:
    setText( 2, i18n( "Home" ) );
    break;
    case Work:
    setText( 2, i18n( "Work" ) );
    break;
    case Any:
    setText( 2, i18n( "Any" ) );
    break;
    }
*/
}

void IMAddressLVI::setProtocol( const KPluginInfo &protocol )
{
  mProtocol = protocol;

  setIcon( 0, SmallIcon( mProtocol.icon() ) );
  setText( 0, mProtocol.name() );
}

KPluginInfo IMAddressLVI::protocol() const
{
  return mProtocol;
}

IMContext IMAddressLVI::context() const
{
  return mContext;
}

QString IMAddressLVI::address() const
{
  return mAddress;
}

/*===========================================================================*/

IMEditorWidget::IMEditorWidget( QWidget *parent, const QString &preferredIM )
  : KDialog( parent ), mReadOnly( false )
{
  setCaption( i18n( "Edit Instant Messaging Address" ) );
  setButtons( Help | Ok | Cancel );
  setDefaultButton( Ok );
  QWidget *w = new QWidget(this);
  mWidget = new Ui_IMEditorBase();
  mWidget->setupUi( w );
  setMainWidget(w);
  connect( mWidget->btnAdd, SIGNAL( clicked() ), SLOT( slotAdd() ) );
  connect( mWidget->btnEdit, SIGNAL( clicked() ), SLOT( slotEdit() ) );
  connect( mWidget->btnDelete, SIGNAL( clicked() ), SLOT( slotDelete() ) );
  connect( mWidget->btnSetStandard, SIGNAL( clicked()), SLOT( slotSetStandard() ) );
  connect( mWidget->lvAddresses, SIGNAL( itemSelectionChanged() ), SLOT( slotUpdateButtons() ) );

  connect( mWidget->lvAddresses, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
           SLOT( slotEdit() ) );

  setHelp( "managing-contacts-im-addresses", "kaddressbook" );

  mWidget->btnEdit->setEnabled( false );
  mWidget->btnDelete->setEnabled( false );
  mWidget->btnSetStandard->setEnabled( false );
  // Disabled pending implementation
  //mWidget->btnUp->setEnabled( false );
  //mWidget->btnDown->setEnabled( false );
  mPreferred = preferredIM;
  mPreferred = mPreferred.replace( " on ", QString( QChar( 0xE120 ) ), Qt::CaseSensitive );
  mProtocols = KPluginInfo::fromServices(
    KServiceTypeTrader::self()->query( QString::fromLatin1( "KABC/IMProtocol" ) ) );

  // order the protocols by putting them in a qmap, then sorting
  // the set of keys and recreating the list
  QMap<QString, KPluginInfo> protocolMap;
  QList<KPluginInfo>::ConstIterator it;
  QList<KPluginInfo> sorted;
  for ( it = mProtocols.begin(); it != mProtocols.end(); ++it ) {
    protocolMap.insert( it->name(), (*it) );
  }

  QStringList keys = protocolMap.keys();
  keys.sort();
  QStringList::ConstIterator keyIt = keys.begin();
  QStringList::ConstIterator end = keys.end();
  for ( ; keyIt != end; ++keyIt ) {
    sorted.append( protocolMap[*keyIt] );
  }
  mProtocols = sorted;
}

QList<KPluginInfo> IMEditorWidget::availableProtocols() const
{
  return mProtocols;
}

void IMEditorWidget::loadContact( KABC::Addressee *addr )
{
  if ( mWidget->lvAddresses ) {
    mWidget->lvAddresses->clear();
  }

  // see README for details of how Evolution stores IM addresses (differently)
  const QStringList customs = addr->customs();

  QStringList::ConstIterator it;
  bool isSet = false;
  for ( it = customs.begin(); it != customs.end(); ++it ) {
    QString app, name, value;
    splitField( *it, app, name, value );

    if ( app.startsWith( QString::fromLatin1( "messaging/" ) ) ) {
      if ( name == QString::fromLatin1( "All" ) ) {
        const KPluginInfo protocol = protocolFromString( app );
        if ( protocol.isValid() ) {
          QStringList addresses = value.split( QChar( 0xE000 ), QString::SkipEmptyParts );
          QStringList::iterator end = addresses.end();
          for ( QStringList::ConstIterator it = addresses.begin(); it != end; ++it ) {
            IMAddressLVI *imaddresslvi =
              new IMAddressLVI( mWidget->lvAddresses, protocol, *it, Any/*, false*/ );
            if ( !isSet && (*it).trimmed().toLower() == mPreferred.trimmed().toLower() ) {
              imaddresslvi->setPreferred( true );
              isSet = true;  //Only set one item to be preferred
            }
          }
        } else {
          kDebug( 5720 ) <<" no protocol found for:" << app;
        }
      }
    }
  }

  if ( mWidget->lvAddresses->topLevelItem( 0 ) ) {
    mWidget->lvAddresses->topLevelItem( 0 )->setSelected( true );
  }
}

void IMEditorWidget::storeContact( KABC::Addressee *addr )
{
  // for each changed protocol, write a new custom field containing the current set of
  // addresses
  QList<KPluginInfo>::ConstIterator protocolIt;
  for ( protocolIt = mChangedProtocols.begin();
        protocolIt != mChangedProtocols.end(); ++protocolIt ) {
    QStringList lst;
    QTreeWidgetItemIterator addressIt( mWidget->lvAddresses );
    while ( *addressIt ) {
      IMAddressLVI *currentAddress = static_cast<IMAddressLVI*>( *addressIt );
      if ( currentAddress && currentAddress->protocol() == *protocolIt ) {
        lst.append( currentAddress->address() );
      }
      ++addressIt;
    }

    QString addrBookField = protocolIt->property( "X-KDE-InstantMessagingKABCField" ).toString();
    if ( !lst.isEmpty() ) {
      addr->insertCustom( addrBookField, QString::fromLatin1( "All" ),
                          lst.join( QString( 0xE000 ) ) );
    } else {
      addr->removeCustom( addrBookField, QString::fromLatin1( "All" ) );
    }
  }
}

void IMEditorWidget::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
  slotUpdateButtons();
}

void IMEditorWidget::slotSetStandard()
{
  QTreeWidgetItemIterator it( mWidget->lvAddresses, QTreeWidgetItemIterator::Selected );

  // Just set the first one selected as standard
  if ( IMAddressLVI *current = static_cast<IMAddressLVI*>( *it ) ) {
    QTreeWidgetItemIterator it2( mWidget->lvAddresses );
    while ( *it2 ) {
      IMAddressLVI *item = static_cast<IMAddressLVI*>( *it2 );

      if ( item->preferred() ) {
        if ( current == item ) {
          return; //Selected is already preferred
        } else {
          item->setPreferred( false );
          break;
        }
      }

      ++it2;
    }

    mPreferred = current->address();
    current->setPreferred( true );
    setModified( true );
  }
}

void IMEditorWidget::slotUpdateButtons()
{
  QList<QTreeWidgetItem*> selectedItems = mWidget->lvAddresses->selectedItems();
  if ( !mReadOnly && selectedItems.count() == 1 ) {
    mWidget->btnAdd->setEnabled( true );
    mWidget->btnEdit->setEnabled( true );
    mWidget->btnDelete->setEnabled( true );
    IMAddressLVI *current = static_cast<IMAddressLVI*>( selectedItems.first() );

    // Disable "set standard" if already standard
    mWidget->btnSetStandard->setEnabled( !current || !current->preferred() );
  } else if ( !mReadOnly && selectedItems.count() > 1 ) {
    mWidget->btnAdd->setEnabled( true );
    mWidget->btnEdit->setEnabled( false );
    mWidget->btnDelete->setEnabled( true );
    mWidget->btnSetStandard->setEnabled( false );
  } else {
    mWidget->btnAdd->setEnabled( !mReadOnly );
    mWidget->btnSetStandard->setEnabled( false );
    mWidget->btnEdit->setEnabled( false );
    mWidget->btnDelete->setEnabled( false );
  }
}

void IMEditorWidget::setModified( bool modified )
{
  mModified = modified;
}

bool IMEditorWidget::isModified() const
{
  return mModified;
}

void IMEditorWidget::slotAdd()
{
  KDialog addDialog( this );
  addDialog.setCaption( i18nc( "Instant messaging", "Add Address" ) );
  addDialog.setButtons( KDialog::Ok | KDialog::Cancel );
  addDialog.setDefaultButton( KDialog::Ok );
  addDialog.setObjectName( "addaddress" );

  IMAddressWidget *addressWid = new IMAddressWidget( &addDialog, mProtocols );
  addDialog.enableButtonOk( false );
  connect( addressWid, SIGNAL( inValidState( bool ) ),
           &addDialog, SLOT( enableButtonOk( bool ) ) );
  addDialog.setMainWidget( addressWid );

  if ( addDialog.exec() == QDialog::Accepted ) {
    // add the new item
    IMAddressLVI *imaddresslvi =
      new IMAddressLVI( mWidget->lvAddresses, addressWid->protocol(),
                        addressWid->address() /*, addressWid->context() */ );

    // If it's a new address, set is as preferred.
    if ( mPreferred.isEmpty() ) {
      imaddresslvi->setPreferred( true );
      mPreferred = addressWid->address();
    }

    if ( !mChangedProtocols.contains( addressWid->protocol() )  ) {
      mChangedProtocols.append( addressWid->protocol() );
    }

    mWidget->lvAddresses->sortItems( 0, Qt::AscendingOrder );

    setModified( true );
  }
}

void IMEditorWidget::slotEdit()
{
  QTreeWidgetItemIterator it( mWidget->lvAddresses, QTreeWidgetItemIterator::Selected );

  // Just edit the first one selected.
  if ( IMAddressLVI *current = static_cast<IMAddressLVI*>( *it ) ) {
    KDialog editDialog( this );
    editDialog.setCaption( i18nc( "Instant messaging", "Edit Address" ) );
    editDialog.setButtons( KDialog::Ok | KDialog::Cancel );
    editDialog.setDefaultButton( KDialog::Ok );
    editDialog.setObjectName( "editaddress" );
    IMAddressWidget *addressWid = new IMAddressWidget( &editDialog, mProtocols, current->protocol(),
                                                       current->address(), current->context() ) ;
    connect( addressWid, SIGNAL( inValidState( bool ) ),
             &editDialog, SLOT( enableButtonOk( bool ) ) );
    editDialog.setMainWidget( addressWid );

    if ( editDialog.exec() == QDialog::Accepted ) {
      bool modified = false;
      if ( addressWid->address() != current->address() ) {
        modified = true;
        current->setAddress( addressWid->address() );
      }
      if ( addressWid->context() != current->context() ) {
        modified = true;
        current->setContext( addressWid->context() );
      }

      // the entry for the protocol of the current address has changed
      if ( !mChangedProtocols.contains( current->protocol() )  ) {
        mChangedProtocols.append( current->protocol() );
      }
      // update protocol - has another protocol gained an address?
      if ( current->protocol() != addressWid->protocol() ) {
        modified = true;
        // this proto is losing an entry
        current->setProtocol( addressWid->protocol() );
        if ( !mChangedProtocols.contains( current->protocol() ) ) {
          mChangedProtocols.append( current->protocol() );
        }
      }

      if ( modified ) {
        setModified( true );
      }
    }
  }
}

void IMEditorWidget::slotDelete()
{
  int num_selected = 0;

  {
    QTreeWidgetItemIterator it( mWidget->lvAddresses, QTreeWidgetItemIterator::Selected );
    while ( *it ) {
      num_selected++;
      ++it;
    }
  }

  if ( num_selected == 0 ) {
    return;
  }

  if ( KMessageBox::warningContinueCancel(
         this,
         i18ncp( "Instant messaging",
                 "Do you really want to delete the selected address?",
                 "Do you really want to delete the %1 selected addresses?", num_selected ),
         i18n( "Confirm Delete" ), KStandardGuiItem::del() ) != KMessageBox::Continue ) {
    return;
  }

  QTreeWidgetItemIterator it( mWidget->lvAddresses );
  bool deletedPreferred = false;
  while ( *it ) {
    if ( ( *it )->isSelected() ) {
      IMAddressLVI * current = static_cast<IMAddressLVI*>( *it );
      if ( !mChangedProtocols.contains( current->protocol() )  ) {
        mChangedProtocols.append( current->protocol() );
      }

      if ( current->preferred() ) {
        deletedPreferred = true;
      }

      delete current;
    } else {
      ++it;
    }
  }

  if ( deletedPreferred ) {
    IMAddressLVI *first = static_cast<IMAddressLVI*>( mWidget->lvAddresses->topLevelItem( 0 ) );
    if ( first ) {
      first->setPreferred( true );
      mPreferred = first->address();
    } else {
      mPreferred = "";
    }
  }

  setModified( true );
}

QString IMEditorWidget::preferred() const
{
  QString retval( mPreferred );
  return retval.replace( QChar( 0xE120 ), " on " );
}

KPluginInfo IMEditorWidget::protocolFromString( const QString &fieldValue ) const
{
  QList<KPluginInfo>::ConstIterator it;
  for ( it = mProtocols.begin(); it != mProtocols.end(); ++it ) {
    if ( it->property( "X-KDE-InstantMessagingKABCField" ).toString() == fieldValue ) {
      return *it;
    }
  }

  return KPluginInfo();
}

void IMEditorWidget::splitField( const QString &str, QString &app, QString &name, QString &value )
{
  int colon = str.indexOf( ':' );
  if ( colon != -1 ) {
    QString tmp = str.left( colon );
    value = str.mid( colon + 1 );

    int dash = tmp.indexOf( '-' );
    if ( dash != -1 ) {
      app = tmp.left( dash );
      name = tmp.mid( dash + 1 );
    }
  }
}

#include "imeditorwidget.moc"
