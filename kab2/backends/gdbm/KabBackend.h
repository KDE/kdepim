#include <qcstring.h>
#include <qstrlist.h>
#include <kurl.h>

#ifndef KAB_BACKEND_H
#define KAB_BACKEND_H

class KabBackend
{
  public:
    
    KabBackend() {}
    virtual ~KabBackend() {}
    
    virtual bool init   (const KURL &) = 0;
    virtual bool read   (const QCString &, QByteArray &) = 0;
    virtual bool remove (const QCString &) = 0;
    virtual bool write  (const QCString &, const QByteArray &) = 0;
    
    virtual QStrList allKeys() = 0;
};

#endif // Included this file.

