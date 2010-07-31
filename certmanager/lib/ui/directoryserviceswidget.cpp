/*
    directoryserviceswidget.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klar�vdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config.h>

#include "directoryserviceswidget.h"
#include "adddirectoryservicedialogimpl.h"
#include "cryptplugwrapper.h"

#include <klineedit.h>
#include <kleo/cryptoconfig.h>
#include <kiconloader.h>
#include <kdebug.h>

#include <tqbuttongroup.h>
#include <tqtoolbutton.h>
#include <tqlistview.h>
#include <tqpushbutton.h>

using namespace Kleo;

class QX500ListViewItem : public QListViewItem
{
public:
  QX500ListViewItem( TQListView* lv, TQListViewItem* prev,
                     const TQString& serverName,
                     const TQString& portNumber,
                     const TQString& dn,
                     const TQString& username,
                     const TQString& password )
    : TQListViewItem( lv, prev, serverName, portNumber, dn, username ) {
    setPassword( password );
  }

  void setPassword( const TQString& pass ) {
    mPassword = pass;
    setText( 4, pass.isEmpty() ? TQString::null : TQString::fromLatin1( "******" ) );
  }

  const TQString& password() const { return mPassword; }

  void setData( const TQString& serverName,
                const TQString& portNumber,
                const TQString& dn,
                const TQString& username,
                const TQString& password ) {
    setText( 0, serverName );
    setText( 1, portNumber );
    setText( 2, dn );
    setText( 3, username );
    setPassword( password );
  }

  void copyItem( QX500ListViewItem* item ) {
    for ( unsigned int i = 0; i < 4 ; ++i )
      setText( i, item->text( i ) );
    setPassword( item->password() );
  }

private:
  TQString mPassword;
};

Kleo::DirectoryServicesWidget::DirectoryServicesWidget(
  Kleo::CryptoConfigEntry* configEntry,
  TQWidget* parent,  const char* name, WFlags fl )
    : DirectoryServicesWidgetBase( parent, name, fl ),
      mConfigEntry( configEntry )
{
    x500LV->setSorting( -1 );

    // taken from kmail's configuredialog.cpp
    upButton->setIconSet( BarIconSet( "up", KIcon::SizeSmall ) );
    upButton->setEnabled( false ); // b/c no item is selected yet

    downButton->setIconSet( BarIconSet( "down", KIcon::SizeSmall ) );
    downButton->setEnabled( false ); // b/c no item is selected yet
}


/*
 *  Destroys the object and frees any allocated resources
 */
DirectoryServicesWidget::~DirectoryServicesWidget()
{
    // no need to delete child widgets, Qt does it all for us
}


/**
   Enables or disables the widgets in this dialog according to the
   capabilities of the current plugin passed as a parameter.
*/
void DirectoryServicesWidget::enableDisable( CryptPlugWrapper* cryptPlug ) // unused?
{
    // disable the whole page if the plugin does not support the use
    // of directory services
    setEnabled( cryptPlug->hasFeature( Feature_CertificateDirectoryService ) ||
                cryptPlug->hasFeature( Feature_CRLDirectoryService ) );
}


/*
 * protected slot, connected to selectionChanged()
 */
void DirectoryServicesWidget::slotServiceChanged( TQListViewItem* item )
{
    if( item )
        removeServicePB->setEnabled( true );
    else
        removeServicePB->setEnabled( false );
    downButton->setEnabled( item && item->itemBelow() );
    upButton->setEnabled( item && item->itemAbove() );
}


/*
 * protected slot, connected to returnPressed/doubleClicked
 */
void DirectoryServicesWidget::slotServiceSelected( TQListViewItem* item )
{
    AddDirectoryServiceDialogImpl* dlg = new AddDirectoryServiceDialogImpl( this );
    dlg->serverNameED->setText( item->text( 0 ) );
    dlg->portED->setText( item->text( 1 ) );
    dlg->descriptionED->setText( item->text( 2 ) );
    dlg->usernameED->setText( item->text( 3 ) );
    TQString pass = static_cast<QX500ListViewItem *>( item )->password();
    dlg->passwordED->setText( pass );

    if( dlg->exec() == TQDialog::Accepted ) {
        item->setText( 0, dlg->serverNameED->text() );
        item->setText( 1, dlg->portED->text() );
        item->setText( 2, dlg->descriptionED->text() );
        item->setText( 3, dlg->usernameED->text() );
        static_cast<QX500ListViewItem *>( item )->setPassword( dlg->passwordED->text() );
        emit changed();
    }
    delete dlg;
}


/*
 * protected slot
 */
void DirectoryServicesWidget::slotAddService()
{
    AddDirectoryServiceDialogImpl* dlg = new AddDirectoryServiceDialogImpl( this );
    if( dlg->exec() == TQDialog::Accepted ) {
      QX500ListViewItem *item = new QX500ListViewItem( x500LV, x500LV->lastItem(),
                                     dlg->serverNameED->text(),
                                     dlg->portED->text(),
                                     dlg->descriptionED->text(),
                                     dlg->usernameED->text(),
                                     dlg->passwordED->text() );
       slotServiceChanged(item);
        emit changed();
    }
    delete dlg;
}

/*
 * protected slot
 */
void DirectoryServicesWidget::slotDeleteService()
{
    TQListViewItem* item = x500LV->selectedItem();
    Q_ASSERT( item );
    if( !item )
        return;
    else
        delete item;
    x500LV->triggerUpdate();
    item = x500LV->currentItem();
    x500LV->setCurrentItem( item ); // seems necessary...
    x500LV->setSelected( item, true );
    emit changed();
}


void DirectoryServicesWidget::setInitialServices( const KURL::List& urls )
{
    x500LV->clear();
    for( KURL::List::const_iterator it = urls.begin(); it != urls.end(); ++it ) {
        TQString dn = KURL::decode_string( (*it).query().mid( 1 ) ); // decode query and skip leading '?'
        (void)new QX500ListViewItem( x500LV, x500LV->lastItem(),
                                     (*it).host(),
                                     TQString::number( (*it).port() ),
                                     dn,
                                     (*it).user(),
                                     (*it).pass());
    }
}

KURL::List DirectoryServicesWidget::urlList() const
{
    KURL::List lst;
    TQListViewItemIterator it( x500LV );
    for ( ; it.current() ; ++it ) {
        TQListViewItem* item = it.current();
        KURL url;
        url.setProtocol( "ldap" );
        url.setHost( item->text( 0 ) );
        url.setPort( item->text( 1 ).toInt() );
        url.setPath( "/" ); // workaround KURL parsing bug
        url.setQuery( item->text( 2 ) );
        url.setUser( item->text( 3 ) );
        url.setPass( static_cast<QX500ListViewItem *>( item )->password() );
        kdDebug() << url << endl;
        lst << url;
    }
    return lst;
}

void DirectoryServicesWidget::clear()
{
    x500LV->clear();
    emit changed();
}

void DirectoryServicesWidget::load()
{
  if ( mConfigEntry ) {
    setInitialServices( mConfigEntry->urlValueList() );
  }
}

void DirectoryServicesWidget::save()
{
  if ( mConfigEntry ) {
    mConfigEntry->setURLValueList( urlList() );
  }
}

void DirectoryServicesWidget::defaults()
{
  if ( mConfigEntry ) {
    // resetToDefault doesn't work since gpgconf doesn't know any defaults for this entry.
    //mConfigEntry->resetToDefault();
    //load();
    clear(); // the default is an empty list.
  }
}

static void swapItems( QX500ListViewItem *item, QX500ListViewItem *other )
{
  TQString serverName = item->text( 0 );
  TQString portNumber = item->text( 1 );
  TQString dn = item->text( 2 );
  TQString username = item->text( 3 );
  TQString password = item->password();
  item->copyItem( other );
  other->setData( serverName, portNumber, dn, username, password );
}

void Kleo::DirectoryServicesWidget::slotMoveUp()
{
  QX500ListViewItem *item = static_cast<QX500ListViewItem *>( x500LV->selectedItem() );
  if ( !item ) return;
  QX500ListViewItem *above = static_cast<QX500ListViewItem *>( item->itemAbove() );
  if ( !above ) return;
  swapItems( item, above );
  x500LV->setCurrentItem( above );
  x500LV->setSelected( above, true );
  emit changed();
}

void Kleo::DirectoryServicesWidget::slotMoveDown()
{
  QX500ListViewItem *item = static_cast<QX500ListViewItem *>( x500LV->selectedItem() );
  if ( !item ) return;
  QX500ListViewItem *below = static_cast<QX500ListViewItem *>( item->itemBelow() );
  if ( !below ) return;
  swapItems( item, below );
  x500LV->setCurrentItem( below );
  x500LV->setSelected( below, true );
  emit changed();
}

#include "directoryserviceswidget.moc"
