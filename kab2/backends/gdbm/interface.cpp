#include "KabGDBMBackend.h"

extern "C" {
void KabBackendInit(const KURL &);
bool KabBackendRead (const QString &, QByteArray &);
bool KabBackendWrite(const QString &, const QByteArray &);
bool KabBackendRemove(const QString &);
QStringList KabBackendAllKeys();
}

KabGDBMBackend * backend = 0;

  void
KabBackendInit(const KURL & url)
{
  backend = new KabGDBMBackend(url);
}

  bool
KabBackendRead (const QString & s, QByteArray & a)
{
  ASSERT(backend != 0);
  return backend->read(s, a);
}

  bool
KabBackendWrite(const QString & s, const QByteArray & a)
{
  ASSERT(backend != 0);
  return backend->write(s, a);
}

  bool
KabBackendRemove(const QString & s)
{
  ASSERT(backend != 0);
  return backend->remove(s);
}

  QStringList
KabBackendAllKeys()
{
  ASSERT(backend != 0);
  return backend->allKeys();
} 


