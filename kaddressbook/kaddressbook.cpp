/*
 * kaddressbook.cpp
 *
 * Copyright (C) 1999 Don Sanders <dsanders@kde.org>
 */
#include "kaddressbook.h"
#include <kabc/vcardconverter.h>

#include <qkeycode.h>
#include <qregexp.h>
#include <qlayout.h>

#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <kapplication.h>
#include <kprotocolinfo.h>
#include <kprinter.h>
#include <kabc/stdaddressbook.h>
#include <kabc/distributionlistdialog.h>
#include <kabc/distributionlist.h>
#include <kabc/field.h>
#include <kabc/resourceselectdialog.h>

#include "undo.h"
#include "undocmds.h"
#include "addresseeeditordialog.h"
#include "filtereditdialog.h"
#include "importdialog.h"
#include "prefsdialog.h"
#include "ldapsearchdialogimpl.h"
#include "viewmanager.h"
#include "printing/printingwizard.h"
#include "filter.h"
#include "incsearchwidget.h"

KAddressBook::KAddressBook( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  mDocument = KABC::StdAddressBook::self();
  connect( mDocument, SIGNAL( addressBookChanged( AddressBook * ) ),
           SLOT( slotAddressBookChanged() ) );

  mDocument->addCustomField( i18n("Department"), KABC::Field::Organization,
                             "X-Department", "KADDRESSBOOK" );
  mDocument->addCustomField( i18n("Profession"), KABC::Field::Organization,
                             "X-Profession", "KADDRESSBOOK" );
  mDocument->addCustomField( i18n("Assistant's Name"),
                             KABC::Field::Organization,
                             "X-AssistantsName", "KADDRESSBOOK" );
  mDocument->addCustomField( i18n("Manager's Name"),
                             KABC::Field::Organization,
                             "X-ManagersName", "KADDRESSBOOK" );
  mDocument->addCustomField( i18n("Spouse's Name"),
                             KABC::Field::Personal,
                             "X-SpousesName", "KADDRESSBOOK" );
  mDocument->addCustomField( i18n("Office"),
                             KABC::Field::Personal,
                             "X-Office", "KADDRESSBOOK" );
  mDocument->addCustomField( i18n("IM Address"),
                             KABC::Field::Personal,
                             "X-IMAddress", "KADDRESSBOOK" );
  mDocument->addCustomField( i18n("Anniversary"),
                             KABC::Field::Personal,
                             "X-Anniversary", "KADDRESSBOOK" );

  // Create the GUI
  mViewManager = new ViewManager(mDocument, kapp->config(),
                                 this, "mViewManager");
  topLayout->addWidget(mViewManager);
  connect(mViewManager, SIGNAL(selected(const QString &)),
          this, SLOT(addresseeSelected(const QString &)));
  connect(mViewManager, SIGNAL(executed(const QString &)),
          this, SLOT(addresseeExecuted(const QString &)));
  connect(mViewManager, SIGNAL(modified()), this,
          SLOT(viewModified()));
  connect(mViewManager, SIGNAL(importVCard(const QString &, bool)),
          this, SLOT(importVCard(const QString &, bool)));

  mDistEditor = 0;
  mPrefsDialog = 0;
  mLdapSearchDialog = 0;
}


void KAddressBook::newContact()
{/* OLD DCOP CALL  -- still works, but should be newAddressee */
  newAddressee();
}

void KAddressBook::slotDistributionList()
{
  save();
  if (!mDistEditor)
      mDistEditor = new KABC::DistributionListDialog(
	  KABC::StdAddressBook::self(), this );

  mDistEditor->exec();
}

void KAddressBook::addEmail( QString aStr )
{

  QString fullName, email;

  KABC::Addressee::parseEmailAddress(aStr, fullName, email);

  // Try to lookup the addressee matching the email address
  bool found = false;
  QStringList emailList;
  KABC::AddressBook::Iterator iter;
  for (iter = mDocument->begin(); !found && (iter != mDocument->end());
       ++iter)
  {
    emailList = (*iter).emails();
    if (emailList.contains(email) > 0)
    {
      // This changes the name event if the user cancels the edit. I am
      // not sure that is the behaviour that we want, but it was how
      // the original implementation worked.
      found = true;
      (*iter).setNameFromString(fullName);
      editAddressee((*iter).uid());
    }
  }

  if (!found)
  {
    // create a new entry and edit it
    KABC::Addressee a;
    a.setNameFromString(fullName);
    a.insertEmail(email, true);

    mDocument->insertAddressee(a);
    mViewManager->refresh( a.uid() );
    editAddressee(a.uid());
  }
}

ASYNC KAddressBook::showContactEditor ( QString uid )
{ /* OLD DCOP CALL   -- still works, but should be editAddressee */
  editAddressee(uid);
}

void KAddressBook::editAddressee(QString uid)
{
  // First, locate the contact entry
  if (uid == QString::null)
  {
    QStringList uidList = mViewManager->selectedUids();
    if (uidList.count() > 0)
      uid = *(uidList.at(0));
  }

  KABC::Addressee a = mDocument->findByUid(uid);
  if (!a.isEmpty())
  {
    AddresseeEditorDialog *dialog = mEditorDict.find( a.uid() );
    if ( !dialog ) {
      dialog = createAddresseeEditorDialog( this );

      mEditorDict.insert( a.uid(), dialog );

      dialog->setAddressee(a);
    }

    dialog->raise();
    dialog->show();
  }
}

void KAddressBook::newResourceAddressee()
{
  AddresseeEditorDialog *dialog = 0;

  KABC::Resource *resource = KABC::ResourceSelectDialog::getResource( mDocument,
                                                                      this );
  if ( resource ) {
    KABC::Addressee addr;
    addr.setResource( resource );
    dialog = createAddresseeEditorDialog( this );
    dialog->setAddressee( addr );
  } else {
    return;
  }

  mEditorDict.insert( dialog->addressee().uid(), dialog );

  dialog->show();
}

void KAddressBook::newAddressee()
{
  AddresseeEditorDialog *dialog = 0;

  KABC::Addressee addr;
  dialog = createAddresseeEditorDialog( this );
  dialog->setAddressee( addr );

  mEditorDict.insert( dialog->addressee().uid(), dialog );

  dialog->show();
}

AddresseeEditorDialog *KAddressBook::createAddresseeEditorDialog( QWidget *parent,
                                                                  const char *name )
{
  AddresseeEditorDialog *dialog = new AddresseeEditorDialog( parent,
                                                             name ? name : "editorDialog" );
  connect(dialog, SIGNAL(addresseeModified(const KABC::Addressee &)),
          SLOT(addresseeModified(const KABC::Addressee &)));
  connect(dialog, SIGNAL( editorDestroyed( const QString & ) ),
          SLOT( slotEditorDestroyed( const QString & ) ) );

  return dialog;
}

void KAddressBook::slotEditorDestroyed( const QString &uid )
{

  mEditorDict.remove( uid );
}

void KAddressBook::save()
{
  KABC::StdAddressBook *b = dynamic_cast<KABC::StdAddressBook*>(mDocument);
  if (!b || !b->save())
  {
    QString text = i18n("There was an error while attempting to save the "
    			"address book. Please check that some other application is "
    			"not using it. ");

    KMessageBox::error(this, text, i18n("Unable to Save"));
  }

  emit modified(false);
}

void KAddressBook::readConfig()
{
  mViewManager->readConfig();
}

void KAddressBook::writeConfig()
{
  mViewManager->writeConfig();
}

KAddressBook::~KAddressBook()
{
  if (mDistEditor)
      delete mDistEditor;

  writeConfig();

  delete mDocument;
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
  if (!QFile::exists(locateLocal("data", "kabc/std.vcf") )) {
    KMessageBox::sorry( this, i18n("Couldn't find a KDE 2 address book.") );
    return;
  }

  int result = KMessageBox::questionYesNoCancel( this,
      i18n("Override previously imported entries?"),
      i18n("Import KDE 2 Addressbook"));

  if ( !result ) return;

  KProcess proc;

  if ( result == KMessageBox::Yes ) {
    proc << "kab2kabc";
    proc << "--override";
  } else if ( result == KMessageBox::No ) {
    proc << "kab2kabc";
  } else {
    kdDebug() << "KAddressBook::importKDE2(): Unknow return value." << endl;
  }

  proc.start( KProcess::Block );

  mDocument->load();
  mViewManager->refresh();
}

void KAddressBook::importCSV()
{

  ContactImportDialog *dialog = new ContactImportDialog(mDocument, this);

  dialog->exec();

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
    fileName = KFileDialog::getOpenFileName( QString::null,
                                                  "*.vcf|vCards", 0,
                                                  i18n( "Select vCard to Import" ) );
    
  if ( !fileName.isEmpty() ) {
    KABC::VCardConverter converter;
    KABC::Addressee a;
    QFile file( fileName );

    file.open( IO_ReadOnly );
    QByteArray rawData = file.readAll();
    QString data = QString::fromLatin1( rawData.data(), rawData.size() + 1 );
    bool ok = false;
    
    if ( data.contains( "\r\nVERSION:3.0\r\n" ) ) {
      ok = converter.vCardToAddressee( data, a, KABC::VCardConverter::v3_0 );
    } else if ( data.contains( "\r\nVERSION:2.1\r\n" ) ) {
      ok = converter.vCardToAddressee( data, a, KABC::VCardConverter::v2_1 );
    }

    if ( !a.isEmpty() && ok ) {
      // Add it to the document, then let the user edit it. We use a
      // PwNewCommand so the user can undo it.
      PwNewCommand *command = new PwNewCommand(mDocument, a);
      UndoStack::instance()->push(command);
      RedoStack::instance()->clear();

      mViewManager->refresh();

      if ( showDialog )
        editAddressee( a.uid() );

    } else {
      QString text = i18n( "The selected file does not appear to be a valid vCard. "
                           "Please check the file and try again." );

      KMessageBox::sorry( this, text, i18n( "vCard Import Failed" ) );
    }
  }
}

void KAddressBook::exportCSV()
{
  QString fileName = KFileDialog::getSaveFileName("addressbook.csv");

  QFile outFile(fileName);
  if ( outFile.open(IO_WriteOnly) )
  {    // file opened successfully
    QTextStream t( &outFile );        // use a text stream

    KABC::AddressBook::Iterator iter;
    KABC::Field::List fields = mDocument->fields();
    KABC::Field::List::Iterator fieldIter;
    bool first = true;

    // First output the column headings
    for (fieldIter = fields.begin(); fieldIter != fields.end(); ++fieldIter)
    {
      if (!first)
        t << ",";

      t << "\"" << (*fieldIter)->label() << "\"";
      first = false;
    }
    t << "\n";

    // Then all the addressee objects
    KABC::Addressee a;
    for (iter = mDocument->begin(); iter != mDocument->end(); ++iter)
    {
      a = *iter;
      first = true;

      for (fieldIter = fields.begin(); fieldIter != fields.end(); ++fieldIter)
      {
        if (!first)
          t << ",";

        t << "\"" << (*fieldIter)->value( a ).replace( QRegExp("\n"), "\\n" ) << "\"";
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
  KABC::Addressee a;

  QStringList uids = viewManager()->selectedUids();
  if ( uids.count() == 0 )
    return;
  else
    a = mDocument->findByUid( uids[0] );

  if ( a.isEmpty() )
    return;

  QString name = a.givenName() + "_" + a.familyName() + ".vcf";

  QString fileName = KFileDialog::getSaveFileName( name );

  QFile outFile(fileName);
  if ( outFile.open(IO_WriteOnly) )
  {    // file opened successfully
    KABC::VCardConverter converter;
    QString vcard;

    converter.addresseeToVCard( a, vcard );

    QTextStream t( &outFile );        // use a text stream
    t.setEncoding( QTextStream::UnicodeUTF8 );
    t << vcard;

    outFile.close();
  }
}

QString KAddressBook::getNameByPhone( QString phone )
{
  QRegExp r("[/*/-]");

  bool found = false;
  QString ownerName = "";
  KABC::AddressBook::Iterator iter;
  KABC::PhoneNumber::List::Iterator phoneIter;
  KABC::PhoneNumber::List phoneList;
  for (iter = mDocument->begin(); !found && (iter != mDocument->end()); ++iter)
  {
    phoneList = (*iter).phoneNumbers();
    for (phoneIter = phoneList.begin();
         !found && (phoneIter != phoneList.end());
         ++phoneIter)
    {
      // Get rid of separator chars so just the numbers are compared.
      if ((*phoneIter).number().replace( r, "" ) == phone.replace( r, "" ))
      {
      ownerName = (*iter).formattedName();
      found = true;
      }
    }
  }

  return ownerName;
}

void KAddressBook::addresseeSelected(const QString &uid)
{
    emit addresseeSelected(uid != QString::null);
}

void KAddressBook::addresseeExecuted(const QString &uid)
{
  if ( uid != QString::null && !mViewManager->isQuickEditVisible() )
    editAddressee(uid);
}

void KAddressBook::addresseeModified(const KABC::Addressee &a)
{

  Command *command = 0;
  QString uid;

  // check if it exists already
  KABC::Addressee origA = mDocument->findByUid(a.uid());
  if (origA.isEmpty()) {
    // Must be new
    command = new PwNewCommand(mDocument, a);
  } else {
    // Must be an edit
    command = new PwEditCommand(mDocument, origA, a);
    uid = a.uid();
  }
  UndoStack::instance()->push(command);
  RedoStack::instance()->clear();

  mViewManager->refresh(uid);

  emit modified(true);
}

void KAddressBook::viewModified()
{
    emit modified(true);
}
void KAddressBook::configure()
{
  if ( !mPrefsDialog ) {
    mPrefsDialog = new PrefsDialog( this );
    connect(mPrefsDialog, SIGNAL(configChanged()), SLOT(configChanged()));
  }

  // Save the current config so we do not loose anything if the user
  // accepts
  writeConfig();

  mPrefsDialog->show();
  mPrefsDialog->raise();
}

void KAddressBook::slotOpenLDAPDialog()
{
  if( !mLdapSearchDialog ) {
    mLdapSearchDialog = new LDAPSearchDialogImpl( mDocument, this);
    connect( mLdapSearchDialog, SIGNAL( addresseesAdded() ), mViewManager,
            SLOT( refresh() ) );
  } else
    mLdapSearchDialog->rereadConfig();

  if( mLdapSearchDialog->isOK() )
    mLdapSearchDialog->exec();
}

void KAddressBook::print()
{
    // ----- first set up the KPrinter object (rationale: The print
    // styles need the device metrics etc to print correctly, thus the
    // printer has to be selected before the style can be configured:
    KPrinter printer;
    if(!printer.setup(this))
    {
        return;
    }

    KABPrinting::PrintingWizard *wizard=
        KABPrinting::producePrintingWizard(&printer, mDocument,
                                           mViewManager->selectedUids(),
                                           this);
    // remember: the actual printing is done in the overloaded accept
    // slot of the printing wizard (to keep the interface lean and
    // mean), thus you do not need to do anything in case this returns
    // true:
    wizard->exec();
    delete wizard;
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
    it.current()->setAddressee( mDocument->findByUid( it.currentKey() ) );
    ++it;
  }

  mViewManager->refresh();
}

void KAddressBook::configureFilters()
{
  FilterDialog dlg( this );
  
  dlg.setFilters( mViewManager->filters() );

  if ( dlg.exec() )
    mViewManager->filtersChanged( dlg.filters() );
}

void KAddressBook::setIncSearchWidget(IncSearchWidget *w)
{
    mIncSearchWidget=w;
    connect(w, SIGNAL(incSearch(const QString&, int)),
            mViewManager, SLOT(incSearch(const QString&, int)));
    connect(mViewManager, SIGNAL(setIncSearchFields(const QStringList&)),
            w, SLOT(setFields(const QStringList&)));
}

#include "kaddressbook.moc"
