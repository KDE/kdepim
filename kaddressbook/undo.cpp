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

#include "undo.h"

///////////////////////////////
// StackBase

void StackBase::push( Command *c )
{
  mCommandStack.push( c );
  emit changed();
}

bool StackBase::isEmpty()
{
  return mCommandStack.isEmpty();
}

Command *StackBase::top()
{
  return mCommandStack.top();
}

void StackBase::clear()
{
  mCommandStack.clear();
  emit changed();
}

Command *StackBase::pop()
{
  Command *c = mCommandStack.pop();
  if ( c )
    emit changed();

  return c;
}

///////////////////////////////
// UndoStack

UndoStack* UndoStack::instance_ = 0;

UndoStack::UndoStack()
  : StackBase()
{
  // setAutoDelete( true );
}

UndoStack* UndoStack::instance()
{
  if ( !instance_ )
    instance_ = new UndoStack();
  return instance_;
}

void UndoStack::undo()
{
  if ( isEmpty() )
    return;

  Command *command = pop();
  if ( !command->undo() )
    push( command );
  else
    RedoStack::instance()->push( command );
}

////////////////////
// RedoStack

RedoStack* RedoStack::instance_ = 0;

RedoStack::RedoStack()
{
  mCommandStack.setAutoDelete( true );
}

RedoStack* RedoStack::instance()
{
  if ( !instance_ )
    instance_ = new RedoStack();
  return instance_;
}

void RedoStack::redo()
{
  Command *command;
  if ( isEmpty() )
    return;

  command = pop();
  if ( !command->redo() )
    push( command );
  else
    UndoStack::instance()->push( command );
}

#include "undo.moc"
