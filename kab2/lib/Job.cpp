#include "KAddressBookBackend.h"
#include "Job.h"

namespace KAB
{
  Job::Job(JobType t, KAddressBookBackend * backend)
    : type_(t),
      backend_(backend)
  {
  }

  Job::~Job()
  {
  }

    void
  Job::setID(int id)
  {
    id_ = id;
  }

    int
  Job::id() const
  {
    return id_;
  }

    JobType
  Job::type() const
  {
    return type_;
  }

  EntryJob::EntryJob(const QString & entryID, KAddressBookBackend * backend)
    : Job(EntryJobT, backend),
      entryID_(entryID)
  {
  }

    void
  EntryJob::run()
  {
  }

  ContainsJob::ContainsJob(const QString & entryID, KAddressBookBackend * backend)
    : Job(ContainsJobT, backend),
      entryID_(entryID)
  {
  }

    void
  ContainsJob::run()
  {
  }

  EntryListJob::EntryListJob(KAddressBookBackend * backend)
    : Job(EntryListJobT, backend)
  {
  }

    void
  EntryListJob::run()
  {
  }

  InsertJob::InsertJob(const Entry & entry, KAddressBookBackend * backend)
    : Job(InsertJobT, backend),
      entry_(entry)
  {
  }

    void
  InsertJob::run()
  {
  }

  RemoveJob::RemoveJob(const QString & entryID, KAddressBookBackend * backend)
    : Job(RemoveJobT, backend),
      entryID_(entryID)
  {
  }

    void
  RemoveJob::run()
  {
  }

  ReplaceJob::ReplaceJob(const Entry & entry, KAddressBookBackend * backend)
    : Job(ReplaceJobT, backend),
      entry_(entry)
  {
  }

    void
  ReplaceJob::run()
  {
  }
}

