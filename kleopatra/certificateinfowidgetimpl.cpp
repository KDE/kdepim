/* -*- Mode: C -*-

  $Id$

  Copyright (C) 2001 by Klar�lvdalens Datakonsult AB

  GPGMEPLUG is free software; you can redistribute it and/or modify
  it under the terms of GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  GPGMEPLUG is distributed in the hope that it will be useful,
  it under the terms of GNU General Public License as published by
  the Free Software Foundation; version 2 of the License
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
*/

#include <qlistview.h>
#include <qtextedit.h>
#include <qheader.h>
#include <qpushbutton.h>
#include <qcursor.h>
#include <qapplication.h>

#include <klocale.h>
#include <kdialogbase.h>
#include <kmessagebox.h>

#include "certificateinfowidgetimpl.h"
#include "certmanager.h"

CertificateInfoWidgetImpl::CertificateInfoWidgetImpl( CertManager* manager, bool external,
						      QWidget* parent, const char* name )
  : CertificateInfoWidget( parent, name ), _manager(manager), _external( external )
{
  if( !external ) importButton->setEnabled(false);
  listView->setColumnWidthMode( 1, QListView::Manual );
  listView->setResizeMode( QListView::LastColumn );
  QFontMetrics fm = fontMetrics();
  listView->setColumnWidth( 1, fm.width( i18n("Information") ) * 5 );

  listView->header()->setClickEnabled( false );
  listView->setSorting( -1 );

  connect( listView, SIGNAL( selectionChanged( QListViewItem* ) ),
	   this, SLOT( slotShowInfo( QListViewItem* ) ) );
  pathView->setColumnWidthMode( 0, QListView::Manual );
  pathView->setResizeMode( QListView::LastColumn );
  pathView->header()->hide();

  connect( pathView, SIGNAL( doubleClicked( QListViewItem* ) ),
	   this, SLOT( slotShowCertPathDetails( QListViewItem* ) ) );
  connect( pathView, SIGNAL( returnPressed( QListViewItem* ) ),
	   this, SLOT( slotShowCertPathDetails( QListViewItem* ) ) );
  connect( importButton, SIGNAL( clicked() ),
	   this, SLOT( slotImportCertificate() ) );
}

void CertificateInfoWidgetImpl::setCert( const CryptPlugWrapper::CertificateInfo& info )
{
  listView->clear();
  pathView->clear();
  _info = info;

  /* Check if we already have the cert in question */
  if( _manager ) {
    importButton->setEnabled( !_manager->haveCertificate( info.fingerprint ) );
  } else { 
    importButton->setEnabled( false );
  }
  // These will show in the opposite order
  new QListViewItem( listView, i18n("Fingerprint"), info.fingerprint );
  new QListViewItem( listView, i18n("Can be used for certification"), 
		     info.certify?i18n("Yes"):i18n("No") );
  new QListViewItem( listView, i18n("Can be used for encryption"), 
		     info.encrypt?i18n("Yes"):i18n("No") );
  new QListViewItem( listView, i18n("Can be used for signing"), 
		     info.sign?i18n("Yes"):i18n("No") );

  new QListViewItem( listView, i18n("Valid"), QString("From %1 to %2")
		     .arg( info.created.toString() ).arg(info.expire.toString()) );


  //new QListViewItem( listView, i18n("Email"), info.dn["1.2.840.113549.1.9.1"] );
  new QListViewItem( listView, i18n("Country"), info.dn["C"] );
  new QListViewItem( listView, i18n("Organizational Unit"), info.dn["OU"] );
  new QListViewItem( listView, i18n("Organization"), info.dn["O"] );
  new QListViewItem( listView, i18n("Location"), info.dn["L"] );
  new QListViewItem( listView, i18n("Name"), info.dn["CN"] );
  new QListViewItem( listView, i18n("Issuer"), info.issuer.stripWhiteSpace() );

  QStringList::ConstIterator it = info.userid.begin();
  QListViewItem* item = new QListViewItem( listView, i18n("Subject"), 
			    (*it).stripWhiteSpace() );
  ++it;
  while( it != info.userid.end() ) {
    if( (*it)[0] == '<' ) {
      item = new QListViewItem( listView, item, i18n("Email"), (*it).mid(1,(*it).length()-2));
    } else {
      item = new QListViewItem( listView, item, i18n("Aka"), (*it).stripWhiteSpace() );
    }
    ++it;  
  } 

  // Set up cert. path
  if( !_manager ) return;
  const CryptPlugWrapper::CertificateInfoList& lst = _manager->certList();
  QString issuer = info.issuer;
  QStringList items;
  items << info.userid[0];
  while( true ) {
    bool found = false;
    CryptPlugWrapper::CertificateInfo info;
    for( CryptPlugWrapper::CertificateInfoList::ConstIterator it = lst.begin();
	 it != lst.end(); ++it ) {
      if( (*it).userid[0] == issuer && !items.contains( info.userid[0] ) ) {
	info = (*it);
	found = true;
	break;
      }
    }
    if( found ) {
      items.prepend( info.userid[0] );
      issuer = info.issuer;
      // FIXME(steffen): Use real DN comparison
      if( info.userid[0] == info.issuer ) {
	// Root item
	break;
      } 
    } else break;
  }
  item = 0;
  for( QStringList::Iterator it = items.begin(); it != items.end(); ++it ) {
    if( item ) item = new QListViewItem( item, (*it) );
    else item = new QListViewItem( pathView, (*it) );
    item->setOpen( true );
  }
}

void CertificateInfoWidgetImpl::slotShowInfo( QListViewItem* item )
{
  textView->setText( item->text(1) );
}

void CertificateInfoWidgetImpl::slotShowCertPathDetails( QListViewItem* item )
{
  if( !_manager ) return;
  const CryptPlugWrapper::CertificateInfoList& lst = _manager->certList();
  for( CryptPlugWrapper::CertificateInfoList::ConstIterator it = lst.begin();
       it != lst.end(); ++it ) {
    if( (*it).userid[0] == item->text(0) ) {
      KDialogBase* dialog = new KDialogBase( this, "dialog", true, i18n("Additional Information for Key"), KDialogBase::Close, KDialogBase::Close );

      CertificateInfoWidgetImpl* top = new CertificateInfoWidgetImpl( _manager, _manager->isRemote(), dialog );
      dialog->setMainWidget( top );
      top->setCert( *it ); 
      dialog->exec();
      delete dialog;
    }
  }
}



void CertificateInfoWidgetImpl::slotImportCertificate()
{
  if( !_manager ) return;
  QApplication::setOverrideCursor( QCursor::WaitCursor );
  int retval = _manager->importCertificateWithFingerprint( _info.fingerprint );
  QApplication::restoreOverrideCursor();

  if( retval == -42 ) {
    KMessageBox::error( this, i18n("Cryptplug returned success, but no certiftcate was imported.\nYou may need to import the issuer certificate %1 first.").arg( _info.issuer ), i18n("Import error") );    
  } else if( retval ) {
    KMessageBox::error( this, i18n("Error importing certificate.\nCryptPlug returned %1.").arg(retval), i18n("Import error") );
  } else {
    KMessageBox::information( this, i18n("Certificate %1 with fingerprint %2 is imported to the local database.").arg(_info.userid[0]).arg(_info.fingerprint), i18n("Certificate Imported") );
    importButton->setEnabled( false );
  }
}
#include "certificateinfowidgetimpl.moc"
