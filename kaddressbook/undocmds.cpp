#include <qtextstream.h>
#include <qapplication.h>
#include <qclipboard.h>

#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kabc/addressbook.h>

#include "undocmds.h"
#include "addresseeutil.h"
#include "addresseeconfig.h"

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

PwPasteCommand::PwPasteCommand( KABC::AddressBook *doc, 
                                const QString &clipboard )
  : Command(), mDocument(doc), mUidList(), mClipboard(clipboard)
{
  redo();
}

QString PwPasteCommand::name()
{
  return i18n( "Paste" );
}

void PwPasteCommand::undo()
{
  KABC::Addressee a;
  QStringList::Iterator it;
  for( it = mUidList.begin(); it != mUidList.end(); ++it ) 
  {
    a = mDocument->findByUid(*it);
    if (!a.isEmpty())
      mDocument->removeAddressee( a );
  }
  
  mUidList.clear();
}

void PwPasteCommand::redo()
{
  KABC::Addressee::List list = AddresseeUtil::clipboardToAddressees(mClipboard);
  
  KABC::Addressee::List::Iterator iter;
  for (iter = list.begin(); iter != list.end(); ++iter)
  {
    mDocument->insertAddressee(*iter);
    mUidList.append((*iter).uid());
  }
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
