/* This file is part of KDE PIM
   Copyright (C) 1999 Don Sanders <sanders@kde.org>

   License: BSD
*/

#ifndef UNDO_H
#define UNDO_H

#include <qobject.h>
#include <qptrstack.h>
#include <qstring.h>

class Command
{
public:
  Command() {}
  virtual ~Command() {};
  virtual QString name() = 0;
  virtual void redo() = 0; // egcs requires these methods to have
  virtual void undo() = 0; // implementations (Seems like a bug)
                           // pure virtual may not work
};

/** The Undo and Redo stacks now no longer inherit directly from a stack.
* They now contain a stack internally and inherit from StackBase, which
* has a signal for when the stack is modified. This is need to keep
* the edit menu and toolbar up to date.
*
* Really this is a simple observable stack.
*/
class StackBase : public QObject
{
  Q_OBJECT
  
  public:
    StackBase() : QObject() {}
    
    void push(Command *c);
    bool isEmpty();
    Command *top();
    void clear();
    
  signals:
    void changed();
    
  protected:
    /** Protect the pop method so users must call undo/redo to properly
    * use the stack, however the subclasses need it to modify the stack.
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
