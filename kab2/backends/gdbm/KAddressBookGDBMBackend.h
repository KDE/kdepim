#include <gdbm.h>
#include <qstring.h>
#include <qstringlist.h>
#include <kurl.h>

#include "KabBackend.h"

#ifndef KAB_GDBM_BACKEND_H
#define KAB_GDBM_BACKEND_H

class KabGDBMBackend : public KabBackend
{
  public:
    
    KabGDBMBackend(const KURL &);
    virtual ~KabGDBMBackend();
    
    virtual bool read   (const QString &, QByteArray &);
    virtual bool write  (const QString &, const QByteArray &);
    virtual bool remove (const QString &);
    
    virtual QStringList allKeys();
    
  private:

    GDBM_FILE dbf_;
    QString filename_;
};

#endif // Included this file.

