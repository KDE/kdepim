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
    class Iterator
    {
      public:
        Iterator() {}
        Iterator( const Addressee::List::Iterator &it ) : mIt( it ) {}
      
        const Addressee & operator*() const { return *mIt; }
        Addressee &operator*() { return *mIt; }
        Iterator &operator++() { mIt++; return *this; }
        Iterator &operator++(int) { mIt++; return *this; }
        Iterator &operator--() { mIt--; return *this; }
        Iterator &operator--(int) { mIt--; return *this; }
        bool operator==( const Iterator &it ) { return ( mIt == it.mIt ); }
        bool operator!=( const Iterator &it ) { return ( mIt != it.mIt ); }

        Addressee::List::Iterator mIt;
    };
    
    class ConstIterator
    {
      public:
        ConstIterator() {}
        ConstIterator( const Addressee::List::ConstIterator &it ) : mIt( it ) {}
      
        const Addressee & operator*() const { return *mIt; }
        ConstIterator &operator++() { mIt++; return *this; }
        ConstIterator &operator++(int) { mIt++; return *this; }
        ConstIterator &operator--() { mIt--; return *this; }
        ConstIterator &operator--(int) { mIt--; return *this; }
        bool operator==( const ConstIterator &it ) { return ( mIt == it.mIt ); }
        bool operator!=( const ConstIterator &it ) { return ( mIt != it.mIt ); }

        Addressee::List::ConstIterator mIt;
    };
    
    class Ticket
    {
        friend AddressBook;
    
        Ticket( const QString &_fileName ) : fileName( _fileName ) {}
        
        QString fileName;
    };

    AddressBook();
    virtual ~AddressBook();

    Ticket *requestSave( const QString &fileName );
    
    bool load( const QString &fileName );
    bool save( Ticket *ticket );
    
    Iterator begin() { return Iterator( mAddressees.begin() ); }
    ConstIterator begin() const { return ConstIterator( mAddressees.begin() ); }
    Iterator end() { return Iterator( mAddressees.end() ); }
    ConstIterator end() const { return ConstIterator( mAddressees.end() ); }
    void clear();
    
    void insertAddressee( const Addressee & );
    void removeAddressee( const Addressee & );
    void removeAddressee( const Iterator & );

    Iterator find( const Addressee & );

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
