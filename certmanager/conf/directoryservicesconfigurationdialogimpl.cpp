/*
    directoryservicesconfigurationdialogimpl.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klarälvdalens Datakonsult AB

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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
#include "directoryservicesconfigurationdialogimpl.h"
#include "adddirectoryservicedialogimpl.h"
#include "cryptplugwrapper.h"

#include <qlistview.h>
#include <qpushbutton.h>
#include <klineedit.h>
#include <qbuttongroup.h>
#include <kdebug.h>

/*
 *  Constructs a DirectoryServicesConfigurationDialogImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
DirectoryServicesConfigurationDialogImpl::DirectoryServicesConfigurationDialogImpl( QWidget* parent,  const char* name, WFlags fl )
    : DirectoryServicesConfigurationDialog( parent, name, fl )
{
    x500LV->setSorting( -1 );
}


/*
 *  Destroys the object and frees any allocated resources
 */
DirectoryServicesConfigurationDialogImpl::~DirectoryServicesConfigurationDialogImpl()
{
    // no need to delete child widgets, Qt does it all for us
}


/**
   Enables or disables the widgets in this dialog according to the
   capabilities of the current plugin passed as a parameter.
*/
void DirectoryServicesConfigurationDialogImpl::enableDisable( CryptPlugWrapper* cryptPlug )
{
    // disable the whole page if the plugin does not support the use
    // of directory services
    setEnabled( cryptPlug->hasFeature( Feature_CertificateDirectoryService ) ||
                cryptPlug->hasFeature( Feature_CRLDirectoryService ) );
}


/*
 * protected slot
 */
void DirectoryServicesConfigurationDialogImpl::slotServiceChanged( QListViewItem* item )
{
    if( item )
        removeServicePB->setEnabled( true );
    else
        removeServicePB->setEnabled( false );
}


/*
 * protected slot
 */
void DirectoryServicesConfigurationDialogImpl::slotServiceSelected( QListViewItem* item )
{
    AddDirectoryServiceDialogImpl* dlg = new AddDirectoryServiceDialogImpl( this );
    dlg->serverNameED->setText( item->text( 0 ) );
    dlg->portED->setText( item->text( 1 ) );
    dlg->descriptionED->setText( item->text( 2 ) );
    if( dlg->exec() == QDialog::Accepted ) {
        item->setText( 0, dlg->serverNameED->text() );
        item->setText( 1, dlg->portED->text() );
        item->setText( 2, dlg->descriptionED->text() );
    }
    delete dlg;
}


/*
 * protected slot
 */
void DirectoryServicesConfigurationDialogImpl::slotAddService()
{
    AddDirectoryServiceDialogImpl* dlg = new AddDirectoryServiceDialogImpl( this );
    if( dlg->exec() == QDialog::Accepted ) {
        (void)new QListViewItem( x500LV, x500LV->lastItem(),
                                 dlg->serverNameED->text(),
                                 dlg->portED->text(),
                                 dlg->descriptionED->text() );
        emit changed();
    }
}

/*
 * protected slot
 */
void DirectoryServicesConfigurationDialogImpl::slotDeleteService()
{
    QListViewItem* item = x500LV->selectedItem();
    Q_ASSERT( item );
    if( !item )
        return;
    else
        delete item;
    x500LV->triggerUpdate();
    slotServiceChanged( x500LV->selectedItem() );
    emit changed();
}


void DirectoryServicesConfigurationDialogImpl::setInitialServices( const KURL::List& urls )
{
    x500LV->clear();
    for( KURL::List::const_iterator it = urls.begin(); it != urls.end(); ++it ) {
        QString dn = KURL::decode_string( (*it).query().mid( 1 ) ); // decode query and skip leading '?'
        (void)new QListViewItem( x500LV, x500LV->lastItem(),
                                 (*it).host(),
                                 QString::number( (*it).port() ),
                                 dn );
    }
}

KURL::List DirectoryServicesConfigurationDialogImpl::urlList() const
{
    KURL::List lst;
    QListViewItemIterator it( x500LV );
    for ( ; it.current() ; ++it ) {
        QListViewItem* item = it.current();
        KURL url;
        url.setProtocol( "ldap" );
        url.setHost( item->text( 0 ) );
        url.setPort( item->text( 1 ).toInt() );
        url.setQuery( item->text( 2 ) );
        kdDebug() << url << endl;
        lst << url;
    }
    return lst;
}

void DirectoryServicesConfigurationDialogImpl::clear()
{
    x500LV->clear();
    emit changed();
}

#include "directoryservicesconfigurationdialogimpl.moc"
