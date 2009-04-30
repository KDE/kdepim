/*
    This file is part of KAddressBook.
    Copyright (C) 1999 Don Sanders <sanders@kde.org>
                  2005 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef UNDOCMDS_H
#define UNDOCMDS_H

// Commands for undo/redo functionality.

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QUndoCommand>

#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kabc/distributionlist.h>

#include "kablock.h"

namespace KAB { class Core; }

class Command : public QUndoCommand
{
  public:
    Command( KABC::AddressBook *addressBook ) { mAddressBook = addressBook; }

  protected:
    KABC::AddressBook *addressBook() const { return mAddressBook; }
    KABLock *lock() const { return KABLock::self( mAddressBook ); }
  bool resourceExist( KABC::Resource *resource );
  private:
    KABC::AddressBook* mAddressBook;
};

class DeleteCommand : public Command
{
  public:
    DeleteCommand( KABC::AddressBook *addressBook, const QStringList &uidList );

    virtual QString text() const;
    virtual void undo();
    virtual void redo();

  private:
    KABC::Addressee::List mAddresseeList;
    QStringList mUIDList;
};

class DeleteDistListsCommand : public Command
{
  public:
    DeleteDistListsCommand( KABC::AddressBook *addressBook,
                            const QStringList &uidList );

    virtual ~DeleteDistListsCommand();

    virtual QString text() const;
    virtual void undo();
    virtual void redo();

  private:
    QList<KABC::DistributionList*> mDistributionLists;
    QStringList mUIDList;
};

class PasteCommand : public Command
{
  public:
    PasteCommand( KAB::Core *core,
                  const KABC::Addressee::List &addressees );

    virtual QString text() const;
    virtual void undo();
    virtual void redo();

  private:
    KABC::Addressee::List mAddresseeList;
    KAB::Core *mCore;
};

class CutCommand : public Command
{
  public:
    CutCommand( KABC::AddressBook *addressBook, const QStringList &uidList );

    virtual QString text() const;
    virtual void undo();
    virtual void redo();

  private:
    KABC::Addressee::List mAddresseeList;
    QStringList mUIDList;
    QByteArray mClipText;
    QString mOldText;
};

class NewCommand : public Command
{
  public:
    NewCommand( KABC::AddressBook *addressBook,
                const KABC::Addressee::List &addressees );

    virtual QString text() const;
    virtual void undo();
    virtual void redo();

  private:
    KABC::Addressee::List mAddresseeList;
};

class EditCommand : public Command
{
  public:
    EditCommand( KABC::AddressBook *addressBook, const KABC::Addressee &oldAddressee,
                 const KABC::Addressee &newAddressee );

    virtual QString text() const;
    virtual void undo();
    virtual void redo();

  private:
    KABC::Addressee mOldAddressee;
    KABC::Addressee mNewAddressee;
};

#endif // UNDOCMDS
