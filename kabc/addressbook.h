#ifndef KABC_ADDRESSBOOK_H
#define KABC_ADDRESSBOOK_H

#include <sys/types.h>

#include <qobject.h>

#include "addressee.h"

class QTimer;

namespace KABC {

class Format;

class AddressBook : public QObject
{
    Q_OBJECT
  public:
    class Ticket {
        friend AddressBook;
    
        Ticket( const QString &_fileName ) : fileName( _fileName ) {}
        
        QString fileName;
    };

    AddressBook();
    virtual ~AddressBook();
    
    bool load( const QString &fileName );
    bool save( Ticket *ticket );
    
    void clear();
    
    Ticket *requestSave( const QString &fileName );
    
    void setAddressee( const Addressee & );
    void removeAddressee( const Addressee & );
    Addressee addressee( const Addressee & );
    Addressee::List addressees() const;

    Addressee::List findByName( const QString & );
    Addressee::List findByEmail( const QString & );

    bool lock( const QString &fileName );
    void unlock( const QString &fileName );

    void setFileName( const QString & );
    QString fileName() const;

    void dump() const;

  signals:
    void addressBookChanged( AddressBook * );
    void addressBookLocked( AddressBook * );
    void addressBookUnlocked( AddressBook * );

  protected slots:
    void checkFile();

  private:
    Addressee::List mAddressees;

    Format *mFormat;

    QString mFileName;
    
    QString mLockUniqueName;
    
    QTimer *mFileCheckTimer;
    time_t mChangeTime;
};

}

#endif
