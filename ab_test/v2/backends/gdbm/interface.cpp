#include "KabGDBMBackend.h"

extern "C" {
void KabBackendInit(const KURL &);
bool KabBackendRead (const QCString &, QByteArray &);
bool KabBackendWrite(const QCString &, const QByteArray &);
bool KabBackendRemove(const QCString &);
void KabBackendAllKeys(QStrList &);
}

KabGDBMBackend * backend = 0;

  void
KabBackendInit(const KURL & url)
{
  backend = new KabGDBMBackend;
  backend->init(url);
}

  bool
KabBackendRead (const QCString & s, QByteArray & a)
{
  ASSERT(backend != 0);
  return backend->read(s, a);
}

  bool
KabBackendWrite(const QCString & s, const QByteArray & a)
{
  ASSERT(backend != 0);
  return backend->write(s, a);
}

  void
KabBackendAllKeys(QStrList & l)
{
  ASSERT(backend != 0);
  l = backend->allKeys();
} 

  bool
KabBackendRemove(const QCString & s)
{
  ASSERT(backend != 0);
  return backend->remove(s);
}

