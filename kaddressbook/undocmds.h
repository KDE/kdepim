/*
    This file is part of KAddressBook.
    Copyright (C) 1999 Don Sanders <sanders@kde.org>

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

#include <qstring.h>
#include <qstringlist.h>

#include <kabc/addressee.h>

#include "undo.h"

namespace KAB {
class Core;
}

class PwDeleteCommand : public Command
{
  public:
    PwDeleteCommand( KABC::AddressBook *ab, const QStringList &uidList );
    virtual ~PwDeleteCommand();

    virtual QString name();
    virtual bool undo();
    virtual bool redo();

  private:
    KABC::Addressee::List mAddresseeList;
    QStringList mUIDList;
};

class PwPasteCommand : public Command
{
  public:
    PwPasteCommand( KAB::Core *core, const KABC::Addressee::List &list );

    virtual QString name();
    virtual bool undo();
    virtual bool redo();

  private:
    KAB::Core *mCore;
    KABC::Addressee::List mAddresseeList;
};

class PwCutCommand : public Command
{
  public:
    PwCutCommand( KABC::AddressBook *ab, const QStringList &uidList );

    virtual QString name();
    virtual bool undo();
    virtual bool redo();

  private:
    KABC::Addressee::List mAddresseeList;
    QStringList mUIDList;
    QString mClipText;
    QString mOldText;
};

class PwNewCommand : public Command
{
  public:
    PwNewCommand( KABC::AddressBook *ab, const KABC::Addressee &a );
    ~PwNewCommand();

    virtual QString name();
    virtual bool undo();
    virtual bool redo();

  private:
    KABC::Addressee mAddr;
};

class PwEditCommand : public Command
{
  public:
    PwEditCommand( KABC::AddressBook *ab, const KABC::Addressee &oldAddr,
                   const KABC::Addressee &newAddr );
    virtual ~PwEditCommand();

    virtual QString name();
    virtual bool undo();
    virtual bool redo();

  private:
    KABC::Addressee mOldAddr;
    KABC::Addressee mNewAddr;
};

#endif
