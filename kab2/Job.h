#ifndef JOB_H
#define JOB_H

#include <kab2/Entry.h>

class KAddressBookBackend;

namespace KAB
{
  enum JobType
  {
    EntryJobT,
    EntryListJobT,
    ContainsJobT,
    InsertJobT,
    RemoveJobT,
    ReplaceJobT
  };

  class Job
  {
    public:

      Job(JobType, KAddressBookBackend * backend);

      virtual ~Job();

      virtual void run() = 0;

      void setID(int);

      int id() const;

      JobType type() const;

    private:

      int id_;

      JobType type_;

      KAddressBookBackend * backend_;
  };

  class EntryJob : public Job
  {
    public:

      EntryJob(const QString & entryID, KAddressBookBackend * backend);

      virtual void run();

    private:

      QString entryID_;
  };

  class ContainsJob : public Job
  {
    public:

      ContainsJob(const QString & entryID, KAddressBookBackend * backend);

      virtual void run();

    private:

      QString entryID_;
  };


  class EntryListJob : public Job
  {
    public:

      EntryListJob(KAddressBookBackend * backend);

      virtual void run();
  };

  class InsertJob : public Job
  {
    public:

      InsertJob(const Entry & entry, KAddressBookBackend * backend);

      virtual void run();

    private:

      Entry entry_;
  };

  class RemoveJob : public Job
  {
    public:

      RemoveJob(const QString & entryID, KAddressBookBackend * backend);

      virtual void run();

    private:

      QString entryID_;
  };

  class ReplaceJob : public Job
  {
    public:

      ReplaceJob(const Entry & entry, KAddressBookBackend * backend);

      virtual void run();

    private:

      Entry entry_;
  };

} // End namespace KAB.

#endif

