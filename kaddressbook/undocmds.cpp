#include <qtextstream.h>
#include <qapplication.h>
#include <qclipboard.h>

#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kabc/addressbook.h>

#include "addresseeutil.h"
#include "addresseeconfig.h"
#include "kabcore.h"

#include "undocmds.h"

/////////////////////////////////
// PwDelete Methods

PwDeleteCommand::PwDeleteCommand(KABC::AddressBook *doc, 
                                 const QStringList &uidList)
  : Command(), mDocument(doc), mAddresseeList(), mUidList(uidList)
{
  redo();
}

PwDeleteCommand::~PwDeleteCommand()
{
}

QString PwDeleteCommand::name()
{
  return i18n( "Delete" );
}

void PwDeleteCommand::undo()
{
  // Put it back in the document
  KABC::Addressee::List::Iterator iter;
  for (iter = mAddresseeList.begin(); iter != mAddresseeList.end(); ++iter)
  {
    mDocument->insertAddressee(*iter);
  }
  
  mAddresseeList.clear();
}

void PwDeleteCommand::redo()
{
  // Just remove it from the document. This is enough to make the user
  // Think the item has been deleted
  KABC::Addressee a;
  QStringList::Iterator iter;
  for (iter = mUidList.begin(); iter != mUidList.end(); ++iter)
  {
    a = mDocument->findByUid(*iter);
    mDocument->removeAddressee(a);
    mAddresseeList.append(a);
    AddresseeConfig cfg(a);
    cfg.remove();
  }
}

/////////////////////////////////
// PwPaste Methods

PwPasteCommand::PwPasteCommand( KABCore *core, const KABC::Addressee::List &list )
  : Command(), mCore( core ), mAddresseeList( list )
{
  redo();
}

QString PwPasteCommand::name()
{
  return i18n( "Paste" );
}

void PwPasteCommand::undo()
{
  KABC::Addressee::List::Iterator it;
  for ( it = mAddresseeList.begin(); it != mAddresseeList.end(); ++it ) 
    mCore->addressBook()->removeAddressee( *it );
}

void PwPasteCommand::redo()
{
  QStringList uids;
  KABC::Addressee::List::Iterator it;
  for ( it = mAddresseeList.begin(); it != mAddresseeList.end(); ++it ) {
    /* we have to set a new uid for the contact, otherwise insertAddressee()
       ignore it.
     */ 
    (*it).setUid( KApplication::randomString( 10 ) );
    uids.append( (*it).uid() );
    mCore->addressBook()->insertAddressee( *it );
  }

  QStringList::Iterator uidIt;
  for ( uidIt = uids.begin(); uidIt != uids.end(); ++uidIt )
    mCore->editContact( *uidIt );
}

/////////////////////////////////
// PwNew Methods

PwNewCommand::PwNewCommand( KABC::AddressBook *doc, const KABC::Addressee &a )
  : Command(), mDocument( doc ), mA( a )
{
  mDocument->insertAddressee(mA);
}

PwNewCommand::~PwNewCommand()
{
}

QString PwNewCommand::name()
{
  return i18n( "New Contact" );
}

void PwNewCommand::undo()
{
  mDocument->removeAddressee( mA );
}

void PwNewCommand::redo()
{
  mDocument->insertAddressee( mA );
}

/////////////////////////////////
// PwEdit Methods

PwEditCommand::PwEditCommand(KABC::AddressBook *doc,
                             const KABC::Addressee &oldA,
                             const KABC::Addressee &newA )
     : Command(), mDocument(doc), mOldA(oldA), mNewA(newA)
{
  redo();
}

PwEditCommand::~PwEditCommand()
{
}

QString PwEditCommand::name()
{
  return i18n( "Entry Edit" );
}

void PwEditCommand::undo()
{
  mDocument->insertAddressee(mOldA);
}

void PwEditCommand::redo()
{
  mDocument->insertAddressee(mNewA);
}

/////////////////////////////////
// PwCut Methods

PwCutCommand::PwCutCommand(KABC::AddressBook *doc, const QStringList &uidList)
    : Command(), mDocument(doc), mAddresseeList(), mUidList(uidList), 
      mClipText(), mOldText()
{
  redo();
}

QString PwCutCommand::name()
{
  return i18n( "Cut" );
}

void PwCutCommand::undo()
{
  KABC::Addressee::List::Iterator iter;
  for (iter = mAddresseeList.begin(); iter != mAddresseeList.end(); ++iter)
  {
    mDocument->insertAddressee(*iter);
  }
  mAddresseeList.clear();
  
  QClipboard *cb = QApplication::clipboard();
  kapp->processEvents();
  cb->setText( mOldText );
}

void PwCutCommand::redo()
{
  KABC::Addressee a;
  QStringList::Iterator iter;
  for (iter = mUidList.begin(); iter != mUidList.end(); ++iter)
  {
    a = mDocument->findByUid(*iter);
    mDocument->removeAddressee(a);
    mAddresseeList.append(a);
  }
  
  // Convert to clipboard
  mClipText = AddresseeUtil::addresseesToClipboard(mAddresseeList);
  
  QClipboard *cb = QApplication::clipboard();
  mOldText = cb->text();
  kapp->processEvents();
  cb->setText( mClipText );
}
