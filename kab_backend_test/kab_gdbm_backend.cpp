#include "kab_gdbm_backend.h"

#include <qfile.h>

  bool
Entity::write(const QCString & key, KabBackend * backend)
{
  QByteArray a;
  
  QDataStream str(a, IO_WriteOnly);
  str << *this;
  
  bool retval =
   backend->write(key, a.data(), a.size());
  
  return retval;
}

  bool
Entity::read(const QCString & key, KabBackend * backend)
{
  char * s;
  unsigned long int l;
  
  if (!backend->read(key, &s, l))
    return false;
  
  QByteArray a(l);
  a.setRawData(s, l);
  
  QDataStream str(a, IO_ReadOnly);
  str >> *this;
  
  a.resetRawData(s, l);
  delete [] s;
  
  return true;
}

  QDataStream &
operator << (QDataStream & str, const Entity & e)
{
  str << e.name_ << e.data_;
  return str;
}
 
  QDataStream &
operator >> (QDataStream & str, Entity & e)
{
  str >> e.name_ >> e.data_;
  return str;
}

KabGDBMBackend::KabGDBMBackend()
  : KabBackend(),
    state_      (Closed),
    blockSize_  (1024),
    mode_       (0600),
    filename_   ("test.gdbm")
{
  if (!QFile(filename_).exists()) {
    _create();
    _close();
  }
}

KabGDBMBackend::~KabGDBMBackend()
{
  _close();
}
  
  bool
KabGDBMBackend::write(
  const QCString & key, const char * value, unsigned long int len)
{
  _openForWriting();

  if (state_ != Write)
    return false;
  
  datum k;
  k.dptr  = key.data();
  k.dsize = key.length();
  
  datum d;
  d.dptr  = (char *)value;
  d.dsize = len;
  
  int ret = gdbm_store(dbf_, k, d, GDBM_REPLACE);
  
  return (ret == 0);
}
  
  bool
KabGDBMBackend::read(
  const QCString & key, char ** value, unsigned long int & len)
{
  _openForReading();

  if (state_ != Read)
    return false;
  
  datum k;
  k.dptr  = (char *)key.data();
  k.dsize = key.length();
  
  datum out = gdbm_fetch(dbf_, k);
  
  *value = out.dptr;
  len    = out.dsize;
  
  return (out.dptr != NULL);

}

  void
KabGDBMBackend::_openForWriting()
{
  if (state_ == Write)
    return;
  
  if (state_ != Closed)
    _close();
  
  dbf_ = gdbm_open(
    filename_.local8Bit().data(), blockSize_, GDBM_WRITER, mode_, NULL);
  
  state_ = (dbf_ == NULL) ? Closed : Write; 
}

  void
KabGDBMBackend::_openForReading()
{
  if (state_ == Read)
    return;

  if (state_ != Closed)
    _close();
  
  dbf_ = gdbm_open(
    filename_.local8Bit().data(), blockSize_, GDBM_READER, mode_, NULL);
  
  state_ = (dbf_ == NULL) ? Closed : Read; 
}

 void
KabGDBMBackend::_create()
{
  if (state_ != Closed)
    _close();
  
  dbf_ = gdbm_open(
    filename_.local8Bit().data(), blockSize_, GDBM_WRCREAT, mode_, NULL);
  
  state_ = (dbf_ == NULL) ? Closed : Write; 
}

  void
KabGDBMBackend::_close()
{
  if (state_ == Closed)
    return;
  
  gdbm_close(dbf_);
  
  state_ = Closed;
}

