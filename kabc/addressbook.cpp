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
#include "vcardformat.h"

#include "addressbook.h"
#include "addressbook.moc"

using namespace KABC;

AddressBook::AddressBook( Format *format )
{
  if ( !format ) {
    mFormat = new VCardFormat();
  } else {
    mFormat = format;
  }

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

AddressBook::Ticket *AddressBook::requestSaveTicket( const QString &fn )
{
  QString saveFileName;

  if ( fn.isEmpty() ) saveFileName = fileName();
  else saveFileName = fn;

  if ( !lock( saveFileName ) ) return 0;
  return new Ticket( saveFileName );
}

void AddressBook::insertAddressee( const Addressee &a )
{
  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( a.uid() == (*it).uid() ) {
      (*it) = a;
      return;
    }
  }
  mAddressees.append( a );
}

void AddressBook::removeAddressee( const Addressee &a )
{
  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( a.uid() == (*it).uid() ) {
      removeAddressee( it );
      return;
    }
  }
}

void AddressBook::removeAddressee( const Iterator &it )
{
  mAddressees.remove( it.mIt );
}

AddressBook::Iterator AddressBook::find( const Addressee &a )
{
  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( a.uid() == (*it).uid() ) {
      return it;
    }
  }
  return end();
}

Addressee::List AddressBook::findByName( const QString &name )
{
  Addressee::List results;

  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( name == (*it).name() ) {
      results.append( *it );
    }
  }

  return results;
}

Addressee::List AddressBook::findByEmail( const QString &email )
{
  Addressee::List results;

  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( email == (*it).preferredEmail() ) {
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

  ConstIterator it;
  for( it = begin(); it != end(); ++it ) {
    (*it).dump();
  }

  kdDebug() << "AddressBook::dump() ---  end  ---" << endl;
}
