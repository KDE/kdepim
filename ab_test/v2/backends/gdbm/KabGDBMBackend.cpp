#include <iostream>
#include "KabGDBMBackend.h"
#include <qfile.h>

KabGDBMBackend::KabGDBMBackend()
  : KabBackend(),
    state_      (Closed),
    blockSize_  (1024),
    mode_       (0600)
{
  if (!QFile(filename_).exists())
    _create();
}

KabGDBMBackend::~KabGDBMBackend()
{
  _close();
}

  bool
KabGDBMBackend::init(const KURL & url)
{
  filename_ = url.path();
  _create();
  return (state_ != Closed);
}
  
  bool
KabGDBMBackend::write(
  const QCString & key, const QByteArray & value)
{
  _openForWriting();

  if (state_ != Write)
    return false;
  
  datum k;
  k.dptr  = key.data();
  k.dsize = key.length();
  
  datum d;
  d.dptr  = value.data();
  d.dsize = value.size();
  
  gdbm_store(dbf_, k, d, GDBM_REPLACE);
  
  return true;
}
  
  bool
KabGDBMBackend::read(
  const QCString & key, QByteArray & a)
{
  _openForReading();

  if (state_ == Closed)
    return false;
  
  datum k;
  k.dptr  = (char *)key.data();
  k.dsize = key.length();
  
  datum out = gdbm_fetch(dbf_, k);
  
  a.setRawData(out.dptr, out.dsize);
  
  return (out.dptr != NULL);
}

  bool
KabGDBMBackend::remove(const QCString & key)
{
  _openForWriting();

  if (state_ != Write)
    return false;
  
  datum k;
  k.dptr  = (char *)key.data();
  k.dsize = key.length();
  
  return (gdbm_delete(dbf_, k) == 0);
}

  void
KabGDBMBackend::_openForWriting()
{
  if (state_ == Write)
    return;
  
  if (state_ == Read)
    _close();
  
  dbf_ = gdbm_open(
    filename_.local8Bit().data(), blockSize_, GDBM_WRITER, mode_, NULL);
  
  state_ = (dbf_ == NULL) ? Closed : Write; 
}

  void
KabGDBMBackend::_openForReading()
{
  if (state_ != Closed)
    return;
  
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

  QStrList
KabGDBMBackend::allKeys()
{
  QStrList l;
  
  _openForReading();
  
  if (state_ == Closed) return l;
  
  l.setAutoDelete(true);
  
  datum key = gdbm_firstkey(dbf_);
  
  while (key.dptr) {
    datum nextkey = gdbm_nextkey(dbf_, key);
    QCString s = key.dptr;
    s.truncate(key.dsize);
    l.append(s);
    key = nextkey;
  }
  
  return l;
}

