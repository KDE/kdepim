#include <gdbm.h>
#include <qstring.h>
#include <qcstring.h>
#include <qstrlist.h>
#include <kurl.h>

#include "KabBackend.h"

#ifndef KAB_GDBM_BACKEND_H
#define KAB_GDBM_BACKEND_H

class KabGDBMBackend : public KabBackend
{
  public:
    
    KabGDBMBackend();
    virtual ~KabGDBMBackend();
    
    virtual bool init   (const KURL &);
    virtual bool read   (const QCString &, QByteArray &);
    virtual bool write  (const QCString &, const QByteArray &);
    virtual bool remove (const QCString &);
    
    virtual QStrList allKeys();
    
  private:
    
    void _openForWriting();
    void _openForReading();
    void _create();
    void _close();
    
    enum State { Read, Write, Closed };
    State state_;
    GDBM_FILE dbf_;
    int blockSize_;
    int mode_;
    QString filename_;
};

#endif // Included this file.

