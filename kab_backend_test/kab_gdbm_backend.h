#include <gdbm.h>
#include <qstring.h>
#include <qdatastream.h>
#include <qcstring.h>

#ifndef KAB_GDBM_BACKEND_H
#define KAB_GDBM_BACKEND_H

class KabBackend;

class Entity
{
  public:
    
    Entity() {}
    ~Entity() {}
    
    void setName(const QString & s) { name_ = s; }
    void setData(const QString & s) { data_ = s; }
    
    QString name() const { return name_; }
    QString data() const { return data_; }
    
    friend QDataStream & operator << (QDataStream & str, const Entity & e);
    friend QDataStream & operator >> (QDataStream & str, Entity & e);
    
    
    bool write(const QCString & key, KabBackend * backend);
    bool read (const QCString & key, KabBackend * backend);
    
  private:
    
    QString name_;
    QString data_;
};

class KabBackend
{
  public:
    
    KabBackend() {}
    virtual ~KabBackend() {}
    
    virtual bool read (const QCString &, char **, unsigned long int &) = 0;
    virtual bool write(const QCString &, const char *, unsigned long int) = 0;
};

class KabGDBMBackend : public KabBackend
{
  public:
    
    KabGDBMBackend();
    virtual ~KabGDBMBackend();
    
    virtual bool write
      (const QCString & key, const char * value, unsigned long int len);

    virtual bool read
      (const QCString & key, char ** value, unsigned long int & len);
    
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

