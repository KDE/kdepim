#ifndef UNDOCMDS_H
#define UNDOCMDS_H
//
// Commands for undo/redo functionality.

#include <qstring.h>
#include <qstringlist.h>

#include <kabc/addressee.h>

#include "undo.h"

namespace KABC { 
  class AddressBook;
}

class KABCore;

class PwDeleteCommand : public Command
{
public:
  PwDeleteCommand(KABC::AddressBook *doc, const QStringList &uidList );
  virtual ~PwDeleteCommand();
  virtual QString name();
  virtual void undo();
  virtual void redo();

private:
  KABC::AddressBook *mDocument;
  KABC::Addressee::List mAddresseeList;
  QStringList mUidList;
};

class PwPasteCommand : public Command
{
  public:
    PwPasteCommand( KABCore *core, const KABC::Addressee::List &list );
    virtual QString name();
    virtual void undo();
    virtual void redo();

  private:
    KABCore *mCore;
    KABC::Addressee::List mAddresseeList;
};

class PwCutCommand : public Command
{
public:
  PwCutCommand(KABC::AddressBook *doc, const QStringList &uidList);
  virtual QString name();
  virtual void undo();
  virtual void redo();

private:
  KABC::AddressBook *mDocument;
  KABC::Addressee::List mAddresseeList;
  QStringList mUidList;
  QString mClipText;
  QString mOldText;
};

class PwNewCommand : public Command
{
public:
  PwNewCommand(KABC::AddressBook *doc, const KABC::Addressee &a );
  ~PwNewCommand();
  virtual QString name();
  virtual void undo();
  virtual void redo();

private:
  KABC::AddressBook *mDocument;
  KABC::Addressee mA;
};

class PwEditCommand : public Command
{
public:
  PwEditCommand(KABC::AddressBook *doc,
                const KABC::Addressee &oldA, 
                const KABC::Addressee &newA);
  virtual ~PwEditCommand();
  virtual QString name();
  virtual void undo();
  virtual void redo();

private:
  KABC::AddressBook *mDocument;
  KABC::Addressee mOldA;
  KABC::Addressee mNewA;
};

#endif
