#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qtimer.h>
#include <qfile.h>
#include <qregexp.h>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include "simpleformat.h"

#include "addressbook.h"

using namespace KABC;

AddressBook::AddressBook()
{
  mFormat = new SimpleFormat();

  mFileCheckTimer = new QTimer( this );
  connect( mFileCheckTimer, SIGNAL( timeout() ), SLOT( checkFile() ) );
}

AddressBook::~AddressBook()
{
  delete mFormat;
}

bool AddressBook::load( const QString &fileName )
{
  mFileName = fileName;

  mAddressees.clear();

  return mFormat->load( this, fileName );
}

bool AddressBook::save( Ticket *ticket )
{
  bool success = mFormat->save( this, ticket->fileName );

  setFileName( ticket->fileName );
  
  delete ticket;
  unlock( mFileName );

  return success;  
}

void AddressBook::clear()
{
  mAddressees.clear();
}

AddressBook::Ticket *AddressBook::requestSave( const QString &fileName )
{
  if ( !lock( fileName ) ) return 0;
  return new Ticket( fileName );
}

Addressee::List AddressBook::addressees() const
{
  return mAddressees;
}

void AddressBook::setAddressee( const Addressee &a )
{
  Addressee::List::Iterator it;
  for ( it = mAddressees.begin(); it != mAddressees.end(); ++it ) {
    if ( a.uid() == (*it).uid() ) {
      (*it) = a;
      return;
    }
  }
  mAddressees.append( a );
}

void AddressBook::removeAddressee( const Addressee &a )
{
  Addressee::List::Iterator it;
  for ( it = mAddressees.begin(); it != mAddressees.end(); ++it ) {
    if ( a.uid() == (*it).uid() ) {
      mAddressees.remove( it );
      return;
    }
  }
}

Addressee AddressBook::addressee( const Addressee &a )
{
  Addressee::List::ConstIterator it;
  for ( it = mAddressees.begin(); it != mAddressees.end(); ++it ) {
    if ( a.uid() == (*it).uid() ) {
      return *it;
    }
  }
  return a;
}

Addressee::List AddressBook::findByName( const QString &name )
{
  Addressee::List results;

  Addressee::List::ConstIterator it;
  for ( it = mAddressees.begin(); it != mAddressees.end(); ++it ) {
    if ( name == (*it).name() ) {
      results.append( *it );
    }
  }

  return results;
}

Addressee::List AddressBook::findByEmail( const QString &email )
{
  Addressee::List results;

  Addressee::List::ConstIterator it;
  for ( it = mAddressees.begin(); it != mAddressees.end(); ++it ) {
    if ( email == (*it).email() ) {
      results.append( *it );
    }
  }

  return results;
}

bool AddressBook::lock( const QString &fileName )
{
  kdDebug() << "AddressBook::lock()" << endl;

  QString fn = fileName;
  fn.replace( QRegExp("/"), "_" );

  QString lockName = locateLocal( "data", "kabc/lock/" + fn + ".lock" );
  kdDebug() << "-- lock name: " << lockName << endl;

  if (QFile::exists( lockName )) return false;

  QString lockUniqueName;
  lockUniqueName = fn + kapp->randomString(8);
  mLockUniqueName = locateLocal( "data", "kabc/lock/" + lockUniqueName );
  kdDebug() << "-- lock unique name: " << mLockUniqueName << endl;

  // Create unique file
  QFile file( mLockUniqueName );
  file.open( IO_WriteOnly );
  file.close();

  // Create lock file
  int result = ::link( mLockUniqueName, lockName );

  if ( result == 0 ) {
    emit addressBookLocked( this );
    return true;
  }
  
  // TODO: check stat
  
  return false;
}

void AddressBook::unlock( const QString &fileName )
{
  QString fn = fileName;
  fn.replace( QRegExp( "/" ), "_" );

  QString lockName = locate( "data", "kabc/lock/" + fn + ".lock" );
  ::unlink( lockName );
  QFile::remove( mLockUniqueName );
  emit addressBookUnlocked( this );
}

void AddressBook::setFileName( const QString &fileName )
{
  mFileName = fileName;

  struct stat s;
  int result = stat( QFile::encodeName( mFileName ), &s );
  if ( result ) {
    mChangeTime  = s.st_ctime;
  }

  mFileCheckTimer->start( 500 );
}

QString AddressBook::fileName() const
{
  return mFileName;
}

void AddressBook::checkFile()
{
  struct stat s;
  int result = stat( QFile::encodeName( mFileName ), &s );
  if ( result && ( mChangeTime != s.st_ctime ) ) {
    mChangeTime  = s.st_ctime;
    load( mFileName );
    emit addressBookChanged( this );
  }
}

void AddressBook::dump() const
{
  kdDebug() << "AddressBook::dump() --- begin ---" << endl;

  Addressee::List a = addressees();
  Addressee::List::Iterator it;
  for( it = a.begin(); it != a.end(); ++it ) {
    (*it).dump();
  }

  kdDebug() << "AddressBook::dump() ---  end  ---" << endl;
}
