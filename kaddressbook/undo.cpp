/* This file is part of KDE PIM
    Copyright (C) 1999 Don Sanders <sanders@kde.org>

    License: BSD
*/

#include "undo.h"

///////////////////////////////
// StackBase

void StackBase::push(Command *c)
{
  mCommandStack.push(c);
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
  if (c)
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
  if (!instance_)
    instance_ = new UndoStack();
  return instance_;
}

void UndoStack::undo()
{
  if (isEmpty())
    return;
    
  Command *command = pop();
  command->undo();
  
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
  if (!instance_)
    instance_ = new RedoStack();
  return instance_;
}

void RedoStack::redo()
{
  Command *command;
  if (isEmpty())
    return;
    
  command = pop();
  command->redo();
  UndoStack::instance()->push( command );
}

#include "undo.moc"
