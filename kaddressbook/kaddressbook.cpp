/*
    This file is part of KAddressbook.
    Copyright (c) 1999 Don Sanders <dsanders@kde.org>

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

#include <qlayout.h>
#include <qkeycode.h>
#include <qptrlist.h>
#include <qregexp.h>

#include <kabc/field.h>
#include <kabc/resource.h>
#include <kabc/stdaddressbook.h>
#include <kabc/vcardconverter.h>
#include <kapplication.h>
#include <kcmultidialog.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <kprocess.h>
#include <kprotocolinfo.h>
#include <kresources/resource.h>
#include <kresources/resourcemanager.h>
#include <kresources/resourceselectdialog.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>

#include "addresseeeditordialog.h"
#include "filtereditdialog.h"
#include "filter.h"
#include "importdialog.h"
#include "ldapsearchdialog.h"
#include "printing/printingwizard.h"
#include "undo.h"
#include "undocmds.h"
#include "viewmanager.h"

#include "kaddressbook.h"

KAddressBook::KAddressBook( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  mAddressBook = KABC::StdAddressBook::self();
  KABC::StdAddressBook::setAutomaticSave( false );

  connect( mAddressBook, SIGNAL( addressBookChanged( AddressBook * ) ),
           SLOT( slotAddressBookChanged() ) );

  mAddressBook->addCustomField( i18n( "Department" ), KABC::Field::Organization,
                                "X-Department", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Profession" ), KABC::Field::Organization,
                                "X-Profession", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Assistant's Name" ), KABC::Field::Organization,
                                "X-AssistantsName", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Manager's Name" ), KABC::Field::Organization,
                                "X-ManagersName", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Spouse's Name" ), KABC::Field::Personal,
                                "X-SpousesName", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Office" ), KABC::Field::Personal,
                                "X-Office", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "IM Address" ), KABC::Field::Personal,
                                "X-IMAddress", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Anniversary" ), KABC::Field::Personal,
                                "X-Anniversary", "KADDRESSBOOK" );

  // Create the GUI
  mViewManager = new ViewManager( mAddressBook, ViewManager::config(), this );
  topLayout->addWidget( mViewManager );
  connect( mViewManager, SIGNAL( selected( const QString& ) ),
           SLOT( addresseeSelected( const QString& ) ) );
  connect( mViewManager, SIGNAL( executed( const QString& ) ),
           SLOT( addresseeExecuted( const QString& ) ) );
  connect( mViewManager, SIGNAL( modified() ), SLOT( viewModified() ) );
  connect( mViewManager, SIGNAL( importVCard( const QString&, bool ) ),
           SLOT( importVCard( const QString&, bool ) ) );

  mLdapSearchDialog = 0;
  mConfigureDialog = 0;
}

KAddressBook::~KAddressBook()
{
  writeConfig();

  delete mAddressBook;
  mAddressBook = 0;

  delete mConfigureDialog;
  mConfigureDialog = 0;
}

ViewManager *KAddressBook::viewManager()
{
  return mViewManager;
}

void KAddressBook::newContact()
{/* OLD DCOP CALL  -- still works, but should be newAddressee */
  newAddressee();
}

void KAddressBook::addEmail( QString aStr )
{
  QString fullName, email;

  KABC::Addressee::parseEmailAddress( aStr, fullName, email );

  // Try to lookup the addressee matching the email address
  bool found = false;
  QStringList emailList;
  KABC::AddressBook::Iterator it;
  for ( it = mAddressBook->begin(); !found && (it != mAddressBook->end()); ++it ) {
    emailList = (*it).emails();
    if ( emailList.contains( email ) > 0 ) {
      found = true;
      (*it).setNameFromString( fullName );
      editAddressee( (*it).uid() );
    }
  }

  if ( !found ) {
    KABC::Addressee addr;
    addr.setNameFromString( fullName );
    addr.insertEmail( email, true );

    mAddressBook->insertAddressee( addr );
    mViewManager->refresh( addr.uid() );
    editAddressee( addr.uid() );
  }
}

ASYNC KAddressBook::showContactEditor ( QString uid )
{ /* OLD DCOP CALL   -- still works, but should be editAddressee */
  editAddressee( uid );
}

void KAddressBook::editAddressee( QString uid )
{
  if ( mViewManager->isQuickEditVisible() )
    return;

  // First, locate the contact entry
  if ( uid == QString::null ) {
    QStringList uidList = mViewManager->selectedUids();
    if ( uidList.count() > 0 )
      uid = *( uidList.at( 0 ) );
  }

  KABC::Addressee addr = mAddressBook->findByUid( uid );
  if ( !addr.isEmpty() ) {
    AddresseeEditorDialog *dialog = mEditorDict.find( addr.uid() );
    if ( !dialog ) {
      dialog = createAddresseeEditorDialog( this );

      mEditorDict.insert( addr.uid(), dialog );

      dialog->setAddressee( addr );
    }

    dialog->raise();
    dialog->show();
  }
}

void KAddressBook::newAddressee()
{
  AddresseeEditorDialog *dialog = 0;

  QPtrList<KABC::Resource> kabcResources = mAddressBook->resources();

  QPtrList<KRES::Resource> kresResources;
  QPtrListIterator<KABC::Resource> it( kabcResources );
  KABC::Resource *resource;
  while ( ( resource = it.current() ) != 0 ) {
    ++it;
    if ( !resource->readOnly() ) {
      KRES::Resource *res = static_cast<KRES::Resource*>( resource );
      if ( res )
        kresResources.append( res );
    }
  }

  KRES::Resource *res = KRES::ResourceSelectDialog::getResource( kresResources, this );
  resource = static_cast<KABC::Resource*>( res );

  if ( resource ) {
    KABC::Addressee addr;
    addr.setResource( resource );
    dialog = createAddresseeEditorDialog( this );
    dialog->setAddressee( addr );
  } else
    return;

  mEditorDict.insert( dialog->addressee().uid(), dialog );

  dialog->show();
}

AddresseeEditorDialog *KAddressBook::createAddresseeEditorDialog( QWidget *parent,
                                                                  const char *name )
{
  AddresseeEditorDialog *dialog =
          new AddresseeEditorDialog( mViewManager, parent,
                                     name ? name : "editorDialog" );

  connect( dialog, SIGNAL( addresseeModified( const KABC::Addressee& ) ),
           SLOT( addresseeModified( const KABC::Addressee& ) ) );
  connect( dialog, SIGNAL( editorDestroyed( const QString& ) ),
           SLOT( slotEditorDestroyed( const QString& ) ) );

  return dialog;
}

void KAddressBook::slotEditorDestroyed( const QString &uid )
{
  mEditorDict.remove( uid );
}

void KAddressBook::save()
{
  KABC::StdAddressBook *b = dynamic_cast<KABC::StdAddressBook*>( mAddressBook );
  if ( !b || !b->save() ) {
    QString text = i18n( "There was an error while attempting to save the "
    			"address book. Please check that some other application is "
    			"not using it. " );

    KMessageBox::error( this, text, i18n( "Unable to Save" ) );
  }

  emit modified( false );
}

void KAddressBook::readConfig()
{
  mViewManager->readConfig();
}

void KAddressBook::writeConfig()
{
  mViewManager->writeConfig();
}

void KAddressBook::undo()
{
  UndoStack::instance()->undo();

  // Refresh the view
  mViewManager->refresh();
}

void KAddressBook::redo()
{
  RedoStack::instance()->redo();

  // Refresh the view
  mViewManager->refresh();
}

void KAddressBook::importKDE2()
{
  if ( !QFile::exists( locateLocal( "data", "kabc/std.vcf" ) ) ) {
    KMessageBox::sorry( this, i18n( "Couldn't find a KDE 2 address book." ) );
    return;
  }

  int result = KMessageBox::questionYesNoCancel( this,
      i18n( "Override previously imported entries?" ),
      i18n( "Import KDE 2 Addressbook" ) );

  if ( !result ) return;

  KProcess proc;

  if ( result == KMessageBox::Yes ) {
    proc << "kab2kabc";
    proc << "--override";
  } else if ( result == KMessageBox::No )
    proc << "kab2kabc";
  else
    kdDebug(5720) << "KAddressBook::importKDE2(): Unknow return value." << endl;

  proc.start( KProcess::Block );

  mAddressBook->load();
  mViewManager->refresh();
}

void KAddressBook::importCSV()
{
  ContactImportDialog *dialog = new ContactImportDialog( mAddressBook, this );

  if ( dialog->exec() )
      mViewManager->refresh();
  delete dialog;

  emit modified( true );
}

void KAddressBook::importVCardSimple()
{
  importVCard( QString::null, true );
}


void KAddressBook::importVCard( const QString &file, bool showDialog )
{
  QString fileName;

  if ( file )
    fileName = file;
  else
    fileName = KFileDialog::getOpenFileName( QString::null, "*.vcf|vCards", 0,
                                             i18n( "Select vCard to Import" ) );

  if ( !fileName.isEmpty() ) {
    KABC::VCardConverter converter;
    KABC::Addressee addr;
    QFile file( fileName );

    file.open( IO_ReadOnly );
    QByteArray rawData = file.readAll();
    QString data = QString::fromUtf8( rawData.data(), rawData.size() + 1 );
    bool ok = false;

    if ( data.contains( "VERSION:3.0" ) ) {
      ok = converter.vCardToAddressee( data, addr, KABC::VCardConverter::v3_0 );
    } else if ( data.contains( "VERSION:2.1" ) ) {
      ok = converter.vCardToAddressee( data, addr, KABC::VCardConverter::v2_1 );
    }

    if ( !addr.isEmpty() && ok ) {
      // Add it to the document, then let the user edit it. We use a
      // PwNewCommand so the user can undo it.
      PwNewCommand *command = new PwNewCommand( mAddressBook, addr );
      UndoStack::instance()->push( command );
      RedoStack::instance()->clear();

      mViewManager->refresh();

      if ( showDialog )
        editAddressee( addr.uid() );

      emit modified( true );
    } else {
      QString text = i18n( "The selected file does not appear to be a valid vCard. "
                           "Please check the file and try again." );

      KMessageBox::sorry( this, text, i18n( "vCard Import Failed" ) );
    }
  }
}

void KAddressBook::exportCSV()
{
  QString fileName = KFileDialog::getSaveFileName( "addressbook.csv" );
  if ( fileName.isEmpty() )
      return;

  QFile outFile( fileName );
  if ( outFile.open( IO_WriteOnly ) ) {
    QTextStream t( &outFile );

    KABC::AddressBook::Iterator iter;
    KABC::Field::List fields = mAddressBook->fields();
    KABC::Field::List::Iterator fieldIter;
    bool first = true;

    // First output the column headings
    for ( fieldIter = fields.begin(); fieldIter != fields.end(); ++fieldIter ) {
      if ( !first )
        t << ",";

      t << "\"" << (*fieldIter)->label() << "\"";
      first = false;
    }
    t << "\n";

    // Then all the addressee objects
    KABC::Addressee addr;
    for ( iter = mAddressBook->begin(); iter != mAddressBook->end(); ++iter ) {
      addr = *iter;
      first = true;

      for ( fieldIter = fields.begin(); fieldIter != fields.end(); ++fieldIter ) {
        if ( !first )
          t << ",";

        t << "\"" << (*fieldIter)->value( addr ).replace( "\n", "\\n" ) << "\"";
        first = false;
      }

      t << "\n";
    }

    outFile.close();
  }
}

void KAddressBook::exportVCard30()
{
  exportVCard( KABC::VCardConverter::v3_0 );
}

void KAddressBook::exportVCard( KABC::VCardConverter::Version )
{
  KABC::Addressee addr;

  QStringList uids = viewManager()->selectedUids();
  if ( uids.count() == 0 )
    return;
  else
    addr = mAddressBook->findByUid( uids[ 0 ] );

  if ( addr.isEmpty() )
    return;

  QString name = addr.givenName() + "_" + addr.familyName() + ".vcf";

  QString fileName = KFileDialog::getSaveFileName( name );

  QFile outFile( fileName );
  if ( outFile.open( IO_WriteOnly ) ) {
    KABC::VCardConverter converter;
    QString vcard;

    converter.addresseeToVCard( addr, vcard );

    QTextStream t( &outFile );
    t.setEncoding( QTextStream::UnicodeUTF8 );
    t << vcard;

    outFile.close();
  }
}

QString KAddressBook::getNameByPhone( QString phone )
{
  QRegExp r( "[/*/-]" );

  bool found = false;
  QString ownerName = "";
  KABC::AddressBook::Iterator iter;
  KABC::PhoneNumber::List::Iterator phoneIter;
  KABC::PhoneNumber::List phoneList;
  for ( iter = mAddressBook->begin(); !found && ( iter != mAddressBook->end() ); ++iter ) {
    phoneList = (*iter).phoneNumbers();
    for ( phoneIter = phoneList.begin(); !found && ( phoneIter != phoneList.end() );
          ++phoneIter) {
      // Get rid of separator chars so just the numbers are compared.
      if ( (*phoneIter).number().replace( r, "" ) == phone.replace( r, "" ) ) {
        ownerName = (*iter).formattedName();
        found = true;
      }
    }
  }

  return ownerName;
}

void KAddressBook::addresseeSelected( const QString &uid )
{
  emit addresseeSelected( uid != QString::null );
}

void KAddressBook::addresseeExecuted( const QString &uid )
{
  if ( uid != QString::null && !mViewManager->isQuickEditVisible() )
    editAddressee( uid );
}

void KAddressBook::addresseeModified( const KABC::Addressee &addr )
{
  Command *command = 0;
  QString uid;

  // check if it exists already
  KABC::Addressee origAddr = mAddressBook->findByUid( addr.uid() );
  if ( origAddr.isEmpty() )
    command = new PwNewCommand( mAddressBook, addr );
  else {
    command = new PwEditCommand( mAddressBook, origAddr, addr );
    uid = addr.uid();
  }

  UndoStack::instance()->push( command );
  RedoStack::instance()->clear();

  mViewManager->refresh( uid );

  emit modified( true );
}

void KAddressBook::viewModified()
{
  emit modified( true );
}
void KAddressBook::configure()
{

  if ( !mConfigureDialog ) {
    mConfigureDialog = new KCMultiDialog( "PIM", this );
    mConfigureDialog->addModule( "PIM/kabconfig.desktop" );
    mConfigureDialog->addModule( "PIM/kabldapconfig.desktop" );
    connect( mConfigureDialog, SIGNAL( applyClicked() ), SLOT( configChanged() ) );
    connect( mConfigureDialog, SIGNAL( okClicked() ), SLOT( configChanged() ) );
  }

  // Save the current config so we do not loose anything if the user accepts
  writeConfig();

  mConfigureDialog->show();
  mConfigureDialog->raise();
}

void KAddressBook::slotOpenLDAPDialog()
{
  if ( !mLdapSearchDialog ) {
    mLdapSearchDialog = new LDAPSearchDialog( mAddressBook, this );
    connect( mLdapSearchDialog, SIGNAL( addresseesAdded() ), mViewManager,
            SLOT( refresh() ) );
  } else
    mLdapSearchDialog->restoreSettings();

  if ( mLdapSearchDialog->isOK() )
    mLdapSearchDialog->exec();
}

void KAddressBook::print()
{
  KPrinter printer;
  if ( !printer.setup( this ) )
    return;

  KABPrinting::PrintingWizard wizard( &printer, mAddressBook,
                                      mViewManager->selectedUids(), this );

  wizard.exec();
}

void KAddressBook::configChanged()
{
  readConfig();
}

void KAddressBook::slotAddressBookChanged()
{
  QDictIterator<AddresseeEditorDialog> it( mEditorDict );
  while ( it.current() ) {
    if ( it.current()->dirty() ) {
      QString text = i18n( "Data has been changed externally. Unsaved "
                           "changes will be lost." );
      KMessageBox::information( this, text );
    }
    it.current()->setAddressee( mAddressBook->findByUid( it.currentKey() ) );
    ++it;
  }

  mViewManager->refresh();
}

void KAddressBook::configureFilters()
{
  FilterDialog dlg( this );

  dlg.setFilters( mViewManager->filters() );

  if ( dlg.exec() )
    mViewManager->setFilters( dlg.filters() );
}

#include "kaddressbook.moc"
