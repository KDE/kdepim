#ifndef COMMAND_H
#define COMMAND_H

#include "Entry.h"

namespace KAB
{
  enum CommandType
  {
    CommandTypeEntry,
    CommandTypeEntryList,
    CommandTypeContains,
    CommandTypeRemove,
    CommandTypeReplace,
    CommandTypeInsert
  };

  class Command
  {
    public:

      Command(CommandType t)
        : type_(t)
        {
        }

      void setID(int id) { id_ = id; }

      int id() const { return id_; }

      CommandType type() const { return type_; }

    private:

      CommandType type_;

      int id_;
  };

  class CommandEntry : public Command
  {
    public:

      CommandEntry(const QString & entryID)
        : Command(CommandTypeEntry),
      entryID_(entryID)
      {
      }

      QString entryID() const { return entryID_; }

      Entry entry() const { return entry_; }
      void setEntry(const Entry & e) { entry_ = e; }

    private:

      QString entryID_;
      Entry entry_;
  };

  class CommandEntryList : public Command
  {
    public:

      CommandEntryList()
        : Command(CommandTypeEntryList)
        {
        }

      QStringList entryList() const { return entryList_; }
      void setEntryList(const QStringList & l) { entryList_ = l; }

    private:

      QStringList entryList_;
  };


  class CommandContains : public Command
  {
    public:

      CommandContains(QString entryID)
        : Command(CommandTypeContains),
      entryID_(entryID)
      {
      }

      QString entryID() const { return entryID_; }

      bool contains() const { return contains_; }
      void setContains(bool b) { contains_ = b; }

    private:

      QString entryID_;
      bool contains_;
  };

  class CommandInsert : public Command
  {
    public:

      CommandInsert(const Entry & e)
        : Command(CommandTypeInsert),
      entry_(e)
      {
      }

      Entry entry() const { return entry_; }

      bool success() const { return ok_; }
      void setSuccess(bool b) { ok_ = b; }

    private:

      Entry entry_;
      bool ok_;
  };

  class CommandReplace : public Command
  {
    public:

      CommandReplace(const Entry & e)
        : Command(CommandTypeReplace),
      entry_(e)
      {
      }

      Entry entry() const { return entry_; }

      bool success() const { return ok_; }
      void setSuccess(bool b) { ok_ = b; }

    private:

      Entry entry_;
      bool ok_;
  };

  class CommandRemove : public Command
  {
    public:

      CommandRemove(QString entryID)
        : Command(CommandTypeRemove),
      entryID_(entryID)
      {
      }

      QString entryID() const { return entryID_; }

      bool success() const { return ok_; }
      void setSuccess(bool b) { ok_ = b; }

    private:

      QString entryID_;
      bool ok_;
  };

}

#endif

