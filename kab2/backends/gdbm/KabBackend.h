#include <qcstring.h>
#include <qstrlist.h>
#include <kurl.h>

#ifndef KAB_BACKEND_H
#define KAB_BACKEND_H

class KabBackend
{
  public:
    
    KabBackend(const KURL & url) { url_ = url; }
    virtual ~KabBackend() {}
    
    virtual bool read   (const QString &, QByteArray &) = 0;
    virtual bool remove (const QString &) = 0;
    virtual bool write  (const QString &, const QByteArray &) = 0;
    
    virtual QStringList allKeys() = 0;
    
    KURL url() { return url_; }
    
  private:
    
    KURL url_;
};

#endif // Included this file.

