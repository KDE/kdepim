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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef UNDO_H
#define UNDO_H

#include <qobject.h>
#include <qptrstack.h>
#include <qstring.h>

#include <kabc/addressbook.h>

#include "kablock.h"

class Command
{
  public:
    Command( KABC::AddressBook *addressBook ) { mAddressBook = addressBook; }
    virtual ~Command() {};

    virtual QString name() = 0;
    virtual bool redo() = 0;
    virtual bool undo() = 0;

  protected:
    KABC::AddressBook *addressBook() const { return mAddressBook; }
    KABLock *lock() const { return KABLock::self( mAddressBook ); }

  private:
    KABC::AddressBook *mAddressBook;
};

/**
  The Undo and Redo stacks now no longer inherit directly from a stack.
  They now contain a stack internally and inherit from StackBase, which
  has a signal for when the stack is modified. This is need to keep
  the edit menu and toolbar up to date.

  Really this is a simple observable stack.
 */
class StackBase : public QObject
{
  Q_OBJECT

  public:
    StackBase() : QObject() {}

    void push( Command *c );
    bool isEmpty();
    Command *top();
    void clear();

  signals:
    void changed();

  protected:
    /**
      Protect the pop method so users must call undo/redo to properly
      use the stack, however the subclasses need it to modify the stack.
     */
    Command *pop();

    QPtrStack<Command> mCommandStack;
};

class UndoStack : public StackBase
{
  public:
    static UndoStack *instance();
    void undo();

  protected:
    UndoStack();
    static UndoStack* instance_;
};

class RedoStack : public StackBase
{
  public:
    static RedoStack *instance();
    void redo();

  protected:
    RedoStack();
    static RedoStack* instance_;
};

#endif
