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

  if (state_ != Write) {
    cerr << "KabGDBMBackend::write(): Addressbook is not open for writing !" << endl;
    return false;
  }
  
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

  if (state_ == Closed) {
    cerr << "KabGDBMBackend::read(): Addressbook is not open !" << endl;
    return false;
  }
  
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

  if (state_ != Write) {
    cerr << "KabGDBMBackend::remove(): Addressbook is not open for writing !" << endl;
    return false;
  }
  
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
  
  if (state_ == Read) {
    cerr << "KabGDBMBackend::_openForWriting(): Addressbook is already open for reading !" << endl;
    _close();
  }
  
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
  cerr << "KabGDBMBackend::allKeys()" << endl;
  QStrList l;
  
  _openForReading();
  
  if (state_ == Closed) {
   cerr << "KabGDBMBackend::allKeys(): Addressbook is closed !" << endl;
   return l;
  }
  
  datum key = gdbm_firstkey(dbf_);
  
  cerr << "KabGDBMBackend::allKeys(): entering while loop" << endl;
  
  while (key.dptr) {
    cerr << "doing gdbm_nextkey" << endl;
    cerr << "key.dptr = " << key.dptr << endl;
    cerr << "key.dsize = " << key.dsize << endl;
    datum nextkey = gdbm_nextkey(dbf_, key);
    cerr << "doing QCString(key.dptr,key.dsize)" << endl;
    QCString s(key.dptr,key.dsize);
    cerr << "doing l.append" << endl;
    l.append(s);
    cerr << "doing key = nextkey" << endl;
    key = nextkey;
    cerr << "KabGDBMBackend::allKeys(): about to do while test again" << endl;
  }
  
  cerr << "KabGDBMBackend::allKeys(): done" << endl;
  return l;
}

