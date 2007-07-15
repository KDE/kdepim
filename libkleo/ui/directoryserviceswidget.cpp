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

#include "directoryserviceswidget.h"
#include "ui_directoryserviceswidgetbase.h"
#include "adddirectoryservicedialogimpl.h"
#include "cryptplugwrapper.h"

#include <klineedit.h>
#include <kleo/cryptoconfig.h>
#include <kiconloader.h>
#include <kdebug.h>

#include <q3buttongroup.h>
#include <QToolButton>
#include <q3listview.h>
#include <QPushButton>

using namespace Kleo;

class QX500ListViewItem : public Q3ListViewItem
{
public:
  QX500ListViewItem( Q3ListView* lv, Q3ListViewItem* prev,
                     const QString& serverName,
                     const QString& portNumber,
                     const QString& dn,
                     const QString& username,
                     const QString& password )
    : Q3ListViewItem( lv, prev, serverName, portNumber, dn, username ) {
    setPassword( password );
  }

  void setPassword( const QString& pass ) {
    mPassword = pass;
    setText( 4, pass.isEmpty() ? QString() : QString::fromLatin1( "******" ) );
  }

  const QString& password() const { return mPassword; }

  void setData( const QString& serverName,
                const QString& portNumber,
                const QString& dn,
                const QString& username,
                const QString& password ) {
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
  QString mPassword;
};

class Kleo::DirectoryServicesWidgetPrivate
{
public:
  DirectoryServicesWidgetPrivate( Kleo::CryptoConfigEntry* config ) : configEntry( config )
  {}
  Kleo::CryptoConfigEntry* configEntry;
  Ui::DirectoryServicesWidgetBase ui;
};

Kleo::DirectoryServicesWidget::DirectoryServicesWidget(
  Kleo::CryptoConfigEntry* configEntry,
  QWidget* parent,  const char* name, Qt::WFlags fl )
    : QWidget( parent, fl ),
      d( new Kleo::DirectoryServicesWidgetPrivate( configEntry ) )
{
    setObjectName(name);
    d->ui.setupUi(this);
    d->ui.x500LV->setSorting( -1 );

    // taken from kmail's configuredialog.cpp
    d->ui.upButton->setIcon( KIcon( "go-up" ) );
    d->ui.upButton->setIconSize( QSize( K3Icon::SizeSmall, K3Icon::SizeSmall ) );
    d->ui.upButton->setEnabled( false ); // b/c no item is selected yet

    d->ui.downButton->setIcon( KIcon( "go-down" ) );
    d->ui.downButton->setIconSize( QSize( K3Icon::SizeSmall, K3Icon::SizeSmall ) );
    d->ui.downButton->setEnabled( false ); // b/c no item is selected yet

    connect( d->ui.addServicePB,SIGNAL( clicked () ),SLOT(slotAddService() ) );
    connect( d->ui.removeServicePB, SIGNAL( clicked() ), SLOT( slotDeleteService() ) );
    connect( d->ui.x500LV, SIGNAL( returnPressed(Q3ListViewItem*) ), SLOT(slotServiceSelected(Q3ListViewItem*) ) );
    connect( d->ui.x500LV, SIGNAL(doubleClicked(Q3ListViewItem*) ),SLOT( slotServiceSelected(Q3ListViewItem*) ) );
    connect( d->ui.x500LV, SIGNAL( selectionChanged(Q3ListViewItem*) ), SLOT(slotServiceChanged(Q3ListViewItem*) ) );
    connect( d->ui.upButton, SIGNAL(clicked() ), SLOT(slotMoveUp() ) );
    connect( d->ui.downButton, SIGNAL( clicked() ), SLOT( slotMoveDown() ) );
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
void DirectoryServicesWidget::slotServiceChanged( Q3ListViewItem* item )
{
    if( item )
        d->ui.removeServicePB->setEnabled( true );
    else
        d->ui.removeServicePB->setEnabled( false );
    d->ui.downButton->setEnabled( item && item->itemBelow() );
    d->ui.upButton->setEnabled( item && item->itemAbove() );
}


/*
 * protected slot, connected to returnPressed/doubleClicked
 */
void DirectoryServicesWidget::slotServiceSelected( Q3ListViewItem* item )
{
    AddDirectoryServiceDialogImpl* dlg = new AddDirectoryServiceDialogImpl( this );
    dlg->serverNameED->setText( item->text( 0 ) );
    dlg->portED->setText( item->text( 1 ) );
    dlg->descriptionED->setText( item->text( 2 ) );
    dlg->usernameED->setText( item->text( 3 ) );
    QString pass = static_cast<QX500ListViewItem *>( item )->password();
    dlg->passwordED->setText( pass );

    if( dlg->exec() == QDialog::Accepted ) {
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
    if( dlg->exec() == QDialog::Accepted ) {
      QX500ListViewItem *item = new QX500ListViewItem( d->ui.x500LV, d->ui.x500LV->lastItem(),
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
    Q3ListViewItem* item = d->ui.x500LV->selectedItem();
    Q_ASSERT( item );
    if( !item )
        return;
    else
        delete item;
    d->ui.x500LV->triggerUpdate();
    item = d->ui.x500LV->currentItem();
    d->ui.x500LV->setCurrentItem( item ); // seems necessary...
    d->ui.x500LV->setSelected( item, true );
    emit changed();
}


void DirectoryServicesWidget::setInitialServices( const KUrl::List& urls )
{
    d->ui.x500LV->clear();
    for( KUrl::List::const_iterator it = urls.begin(); it != urls.end(); ++it ) {
        QString dn = KUrl::fromPercentEncoding( (*it).query().mid( 1 ).toLatin1() ); // decode query and skip leading '?'
        (void)new QX500ListViewItem( d->ui.x500LV, d->ui.x500LV->lastItem(),
                                     (*it).host(),
                                     QString::number( (*it).port() ),
                                     dn,
                                     (*it).user(),
                                     (*it).pass());
    }
}

KUrl::List DirectoryServicesWidget::urlList() const
{
    KUrl::List lst;
    Q3ListViewItemIterator it( d->ui.x500LV );
    for ( ; it.current() ; ++it ) {
        Q3ListViewItem* item = it.current();
        KUrl url;
        url.setProtocol( "ldap" );
        url.setHost( item->text( 0 ) );
        url.setPort( item->text( 1 ).toInt() );
        url.setPath( "/" ); // workaround KUrl parsing bug
        url.setQuery( item->text( 2 ) );
        url.setUser( item->text( 3 ) );
        url.setPass( static_cast<QX500ListViewItem *>( item )->password() );
        kDebug() << url << endl;
        lst << url;
    }
    return lst;
}

void DirectoryServicesWidget::clear()
{
    d->ui.x500LV->clear();
    emit changed();
}

void DirectoryServicesWidget::load()
{
  if ( d->configEntry ) {
    setInitialServices( d->configEntry->urlValueList() );
  }
}

void DirectoryServicesWidget::save()
{
  if ( d->configEntry ) {
    d->configEntry->setURLValueList( urlList() );
  }
}

void DirectoryServicesWidget::defaults()
{
  if ( d->configEntry ) {
    // resetToDefault doesn't work since gpgconf doesn't know any defaults for this entry.
    //d->configEntry->resetToDefault();
    //load();
    clear(); // the default is an empty list.
  }
}

static void swapItems( QX500ListViewItem *item, QX500ListViewItem *other )
{
  QString serverName = item->text( 0 );
  QString portNumber = item->text( 1 );
  QString dn = item->text( 2 );
  QString username = item->text( 3 );
  QString password = item->password();
  item->copyItem( other );
  other->setData( serverName, portNumber, dn, username, password );
}

void Kleo::DirectoryServicesWidget::slotMoveUp()
{
  QX500ListViewItem *item = static_cast<QX500ListViewItem *>( d->ui.x500LV->selectedItem() );
  if ( !item ) return;
  QX500ListViewItem *above = static_cast<QX500ListViewItem *>( item->itemAbove() );
  if ( !above ) return;
  swapItems( item, above );
  d->ui.x500LV->setCurrentItem( above );
  d->ui.x500LV->setSelected( above, true );
  emit changed();
}

void Kleo::DirectoryServicesWidget::slotMoveDown()
{
  QX500ListViewItem *item = static_cast<QX500ListViewItem *>( d->ui.x500LV->selectedItem() );
  if ( !item ) return;
  QX500ListViewItem *below = static_cast<QX500ListViewItem *>( item->itemBelow() );
  if ( !below ) return;
  swapItems( item, below );
  d->ui.x500LV->setCurrentItem( below );
  d->ui.x500LV->setSelected( below, true );
  emit changed();
}

#include "directoryserviceswidget.moc"
