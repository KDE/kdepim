#include <iostream>
#include "KabGDBMBackend.h"
#include <qfile.h>

const int blockSize = 1024;

KabGDBMBackend::KabGDBMBackend(const KURL & _url)
  : KabBackend(_url)
{
  filename_ = url().path();
  dbf_ = gdbm_open(
    filename_.local8Bit().data(), blockSize, GDBM_WRCREAT, 0600, NULL);
}

KabGDBMBackend::~KabGDBMBackend()
{
  if (dbf_)
    gdbm_close(dbf_);
}

  bool
KabGDBMBackend::write(
  const QString & k, const QByteArray & value)
{
  QCString key(k.ascii());
  datum _key;
  _key.dptr  = const_cast<char *>(key.data());
  _key.dsize = key.length() + 1;
  
  datum d;
  d.dptr  = value.data();
  d.dsize = value.size();
  
  gdbm_store(dbf_, _key, d, GDBM_REPLACE);
  return true;
}
  
  bool
KabGDBMBackend::read(
  const QString & k, QByteArray & a)
{
  QCString key(k.ascii());
  
  datum d;
  d.dptr  = const_cast<char *>(key.data());
  d.dsize = key.length() + 1;
  
  datum out = gdbm_fetch(dbf_, d);
  
  a.setRawData(out.dptr, out.dsize);
  
  return (out.dptr != NULL);
}

  bool
KabGDBMBackend::remove(const QString & k)
{
  QCString key(k.ascii());
  datum _key;
  _key.dptr  = const_cast<char *>(key.data());
  _key.dsize = key.length();
  
  return (gdbm_delete(dbf_, _key) == 0);
}

  QStringList
KabGDBMBackend::allKeys()
{
  QStringList l;
  
  datum key = gdbm_firstkey(dbf_);
  
  while (key.dptr) {
    
    datum nextkey = gdbm_nextkey(dbf_, key);
    
    QCString s(key.dptr, key.dsize);
    
    l.append(QString(s));
    
    key = nextkey;
  }
  
  return l;
}

