/*
    qgpgmecryptoconfig.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "qgpgmecryptoconfig.h"
#include <kdebug.h>
#include <kprocio.h>
#include <errno.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <assert.h>
#include <ktempfile.h>
#include <qfile.h>
#include <stdlib.h>
#include <qtextcodec.h>

// Just for the Q_ASSERT in the dtor. Not thread-safe, but who would
// have 2 threads talking to gpgconf anyway? :)
static bool s_duringClear = false;

#define GPGCONF_FLAG_GROUP 1
#define GPGCONF_FLAG_OPTIONAL 2
#define GPGCONF_FLAG_LIST 4
#define GPGCONF_FLAG_RUNTIME 8
#define GPGCONF_FLAG_DEFAULT 16 // fixed default value available
#define GPGCONF_FLAG_DEFAULT_DESC 32 // runtime default value available
#define GPGCONF_FLAG_NOARG_DESC 64 // option with optional arg; special meaning if no arg set
// Change size of mFlags bitfield if adding new values here

QGpgMECryptoConfig::QGpgMECryptoConfig()
 : mComponents( 7 ), mParsed( false )
{
    mComponents.setAutoDelete( true );
}

QGpgMECryptoConfig::~QGpgMECryptoConfig()
{
}

void QGpgMECryptoConfig::runGpgConf( bool showErrors )
{
  // Run gpgconf --list-components to make the list of components

  KProcIO proc( QTextCodec::codecForName( "utf8" ) );
  proc << "gpgconf"; // must be in the PATH
  proc << "--list-components";

  QObject::connect( &proc, SIGNAL( readReady(KProcIO*) ),
                    this, SLOT( slotCollectStdOut(KProcIO*) ) );

  // run the process:
  int rc = 0;
  if ( !proc.start( KProcess::Block ) )
    rc = -1;
  else
    rc = ( proc.normalExit() ) ? proc.exitStatus() : -2 ;

  // handle errors, if any (and if requested)
  if ( showErrors && rc != 0 ) {
    QString wmsg = i18n("<qt>Failed to execute gpgconf:<br>%1</qt>");
    if ( rc == -1 )
        wmsg = wmsg.arg( i18n( "program not found" ) );
    else if ( rc == -2 )
        wmsg = wmsg.arg( i18n( "program cannot be executed" ) );
    else
        wmsg = wmsg.arg( strerror(rc) );
    kdWarning(5150) << wmsg << endl; // to see it from test_cryptoconfig.cpp
    KMessageBox::error(0, wmsg);
  }
  mParsed = true;
}

void QGpgMECryptoConfig::slotCollectStdOut( KProcIO* proc )
{
  QString line;
  int result;
  while( ( result = proc->readln(line) ) != -1 ) {
    //kdDebug(5150) << "GOT LINE:" << line << endl;
    // Format: NAME:DESCRIPTION
    QStringList lst = QStringList::split( ':', line, true );
    if ( lst.count() >= 2 ) {
      mComponents.insert( lst[0], new QGpgMECryptoConfigComponent( this, lst[0], lst[1] ) );
    } else {
      kdWarning(5150) << "Parse error on gpgconf --list-components output: " << line << endl;
    }
  }
}

QStringList QGpgMECryptoConfig::componentList() const
{
  if ( !mParsed )
    const_cast<QGpgMECryptoConfig*>( this )->runGpgConf( true );
  QDictIterator<QGpgMECryptoConfigComponent> it( mComponents );
  QStringList names;
  for( ; it.current(); ++it )
    names.push_back( it.currentKey() );
  return names;
}

Kleo::CryptoConfigComponent* QGpgMECryptoConfig::component( const QString& name ) const
{
  if ( !mParsed )
    const_cast<QGpgMECryptoConfig*>( this )->runGpgConf( false );
  return mComponents.find( name );
}

void QGpgMECryptoConfig::sync( bool runtime )
{
  QDictIterator<QGpgMECryptoConfigComponent> it( mComponents );
  for( ; it.current(); ++it )
    it.current()->sync( runtime );
}

void QGpgMECryptoConfig::clear()
{
  s_duringClear = true;
  mComponents.clear();
  s_duringClear = false;
  mParsed = false; // next call to componentList/component will need to run gpgconf again
}

////

QGpgMECryptoConfigComponent::QGpgMECryptoConfigComponent( QGpgMECryptoConfig*, const QString& name, const QString& description )
  : mGroups( 7 ), mName( name ), mDescription( description )
{
  mGroups.setAutoDelete( true );
  runGpgConf();
}

QGpgMECryptoConfigComponent::~QGpgMECryptoConfigComponent()
{
}

void QGpgMECryptoConfigComponent::runGpgConf()
{
  // Run gpgconf --list-options <component>, and create all groups and entries for that component

  KProcIO proc( QTextCodec::codecForName( "utf8" ) );
  proc << "gpgconf"; // must be in the PATH
  proc << "--list-options";
  proc << mName;

  //kdDebug(5150) << "Running gpgconf --list-options " << mName << endl;

  QObject::connect( &proc, SIGNAL( readReady(KProcIO*) ),
                    this, SLOT( slotCollectStdOut(KProcIO*) ) );
  mCurrentGroup = 0;

  // run the process:
  int rc = 0;
  if ( !proc.start( KProcess::Block ) )
    rc = -1;
  else
    rc = ( proc.normalExit() ) ? proc.exitStatus() : -1 ;

  if( rc != 0 ) // can happen when using the wrong version of gpg...
    kdWarning(5150) << "Running 'gpgconf --list-options " << mName << "' failed. " << strerror( rc ) << ", but try that command to see the real output" << endl;
  else {
    if ( mCurrentGroup && !mCurrentGroup->mEntries.isEmpty() ) // only add non-empty groups
      mGroups.insert( mCurrentGroupName, mCurrentGroup );
  }
}

void QGpgMECryptoConfigComponent::slotCollectStdOut( KProcIO* proc )
{
  QString line;
  int result;
  while( ( result = proc->readln(line) ) != -1 ) {
    //kdDebug(5150) << "GOT LINE:" << line << endl;
    // Format: NAME:FLAGS:LEVEL:DESCRIPTION:TYPE:ALT-TYPE:ARGNAME:DEFAULT:ARGDEF:VALUE
    QStringList lst = QStringList::split( ':', line, true );
    if ( lst.count() >= 10 ) {
      int flags = lst[1].toInt();
      int level = lst[2].toInt();
      if ( level > 2 ) // invisible or internal -> skip it;
        continue;
      if ( flags & GPGCONF_FLAG_GROUP ) {
        if ( mCurrentGroup && !mCurrentGroup->mEntries.isEmpty() ) // only add non-empty groups
          mGroups.insert( mCurrentGroupName, mCurrentGroup );
        //else
        //  kdDebug(5150) << "Discarding empty group " << mCurrentGroupName << endl;
        mCurrentGroup = new QGpgMECryptoConfigGroup( lst[3], level );
        mCurrentGroupName = lst[0];
      } else {
        // normal entry
        if ( !mCurrentGroup ) {  // first toplevel entry -> create toplevel group
          mCurrentGroup = new QGpgMECryptoConfigGroup( QString::null, 0 );
          mCurrentGroupName = "<nogroup>";
        }
        mCurrentGroup->mEntries.insert( lst[0], new QGpgMECryptoConfigEntry( lst ) );
      }
    } else {
      // This happens on lines like
      // dirmngr[31465]: error opening `/home/dfaure/.gnupg/dirmngr_ldapservers.conf': No such file or directory
      // so let's not bother the user with it.
      //kdWarning(5150) << "Parse error on gpgconf --list-options output: " << line << endl;
    }
  }
}

QStringList QGpgMECryptoConfigComponent::groupList() const
{
  QDictIterator<QGpgMECryptoConfigGroup> it( mGroups );
  QStringList names;
  for( ; it.current(); ++it )
    names.push_back( it.currentKey() );
  return names;
}

Kleo::CryptoConfigGroup* QGpgMECryptoConfigComponent::group(const QString& name ) const
{
  return mGroups.find( name );
}

void QGpgMECryptoConfigComponent::sync( bool runtime )
{
  KTempFile tmpFile;
  tmpFile.setAutoDelete( true );

  QValueList<QGpgMECryptoConfigEntry *> dirtyEntries;

  // Collect all dirty entries
  QDictIterator<QGpgMECryptoConfigGroup> groupit( mGroups );
  for( ; groupit.current(); ++groupit ) {
    QDictIterator<QGpgMECryptoConfigEntry> it( groupit.current()->mEntries );
    for( ; it.current(); ++it ) {
      if ( it.current()->isDirty() ) {
        // OK, we can set it.currentKey() to it.current()->outputString()
        QString line = it.currentKey();
        if ( it.current()->isSet() ) { // set option
          line += ":0:";
          line += it.current()->outputString();
        } else {                       // unset option
          line += ":16:";
        }
        line += '\n';
        QCString line8bit = line.utf8(); // encode with utf8, and KProcIO uses utf8 when reading.
        tmpFile.file()->writeBlock( line8bit.data(), line8bit.size()-1 /*no 0*/ );
        dirtyEntries.append( it.current() );
      }
    }
  }
  tmpFile.close();
  if ( dirtyEntries.isEmpty() )
    return;

  // Call gpgconf --change-options <component>
  QString commandLine = "gpgconf";
  if ( runtime )
    commandLine += " --runtime";
  commandLine += " --change-options ";
  commandLine += KProcess::quote( mName );
  commandLine += " < ";
  commandLine += KProcess::quote( tmpFile.name() );

  //kdDebug(5150) << commandLine << endl;
  //system( QCString( "cat " ) + tmpFile.name().latin1() ); // DEBUG

  KProcess proc;
  proc.setUseShell( true );
  proc << commandLine;

  // run the process:
  int rc = 0;
  if ( !proc.start( KProcess::Block ) )
    rc = -1;
  else
    rc = ( proc.normalExit() ) ? proc.exitStatus() : -1 ;

  if ( rc == -1 )
  {
    QString wmsg = i18n( "Could not start gpgconf\nCheck that gpgconf is in the PATH and that it can be started" );
    kdWarning(5150) << wmsg << endl;
    KMessageBox::error(0, wmsg);
  }
  else if( rc != 0 ) // Happens due to bugs in gpgconf (e.g. issues 104/115)
  {
    QString wmsg = i18n( "Error from gpgconf while saving configuration: %1" ).arg( strerror( rc ) );
    kdWarning(5150) << k_funcinfo << ":" << strerror( rc ) << endl;
    KMessageBox::error(0, wmsg);
  }
  else
  {
    QValueList<QGpgMECryptoConfigEntry *>::Iterator it = dirtyEntries.begin();
    for( ; it != dirtyEntries.end(); ++it ) {
      (*it)->setDirty( false );
    }
  }
}

////

QGpgMECryptoConfigGroup::QGpgMECryptoConfigGroup( const QString& description, int level )
  : mEntries( 29 ),
    mDescription( description ),
    mLevel( static_cast<Kleo::CryptoConfigEntry::Level>( level ) )
{
  mEntries.setAutoDelete( true );
}

QStringList QGpgMECryptoConfigGroup::entryList() const
{
  QDictIterator<QGpgMECryptoConfigEntry> it( mEntries );
  QStringList names;
  for( ; it.current(); ++it )
    names.push_back( it.currentKey() );
  return names;
}

Kleo::CryptoConfigEntry* QGpgMECryptoConfigGroup::entry( const QString& name ) const
{
  return mEntries.find( name );
}

////

static QString gpgconf_unescape( const QString& str )
{
  // Looks like it's the same rules as KURL.
  return KURL::decode_string( str, 106 );
}

static QString gpgconf_escape( const QString& str )
{
  // Escape special chars (including ':' and '%')
  QString enc = KURL::encode_string( str, 106 ); // and convert to utf8 first (to get %12%34 for one special char)
  // Also encode commas, for lists.
  enc.replace( ',', "%2c" );
  return enc;
}

static QString urlpart_encode( const QString& str )
{
  QString enc( str );
  enc.replace( '%', "%25" ); // first!
  enc.replace( ':', "%3a" );
  //kdDebug() << "  urlpart_encode: " << str << " -> " << enc << endl;
  return enc;
}

static QString urlpart_decode( const QString& str )
{
  return KURL::decode_string( str );
}

// gpgconf arg type number -> CryptoConfigEntry arg type enum mapping
static Kleo::CryptoConfigEntry::ArgType knownArgType( int argType, bool& ok ) {
  ok = true;
  switch( argType ) {
  case 0: // none
    return Kleo::CryptoConfigEntry::ArgType_None;
  case 1: // string
    return Kleo::CryptoConfigEntry::ArgType_String;
  case 2: // int32
    return Kleo::CryptoConfigEntry::ArgType_Int;
  case 3: // uint32
    return Kleo::CryptoConfigEntry::ArgType_UInt;
  case 32: // pathname
    return Kleo::CryptoConfigEntry::ArgType_Path;
  case 33: // ldap server
    return Kleo::CryptoConfigEntry::ArgType_LDAPURL;
  default:
    ok = false;
    return Kleo::CryptoConfigEntry::ArgType_None;
  }
}

QGpgMECryptoConfigEntry::QGpgMECryptoConfigEntry( const QStringList& parsedLine )
{
  // Format: NAME:FLAGS:LEVEL:DESCRIPTION:TYPE:ALT-TYPE:ARGNAME:DEFAULT:ARGDEF:VALUE
  assert( parsedLine.count() >= 10 ); // called checked for it already
  QStringList::const_iterator it = parsedLine.begin();
  ++it; // skip name, stored in group
  mFlags = (*it++).toInt();
  mLevel = (*it++).toInt();
  mDescription = (*it++);
  bool ok;
  // we keep the real (int) arg type, since it influences the parsing (e.g. for ldap urls)
  mRealArgType = (*it++).toInt();
  mArgType = knownArgType( mRealArgType, ok );
  if ( !ok && !(*it).isEmpty() ) {
    // use ALT-TYPE
    mRealArgType = (*it).toInt();
    mArgType = knownArgType( mRealArgType, ok );
  }
  if ( !ok )
    kdWarning(5150) << "Unsupported datatype: " << parsedLine[4] << " : " << *it << " for " << parsedLine[0] << endl;
  ++it; // done with alt-type
  ++it; // skip argname (not useful in GUIs)

  mSet = false;
  QString value;
  if ( mFlags & GPGCONF_FLAG_DEFAULT ) {
    value = *it; // get default value
    mDefaultValue = stringToValue( value, true );
  }
  ++it; // done with DEFAULT
  ++it; // ### skip ARGDEF for now. It's only for options with an "optional arg"
  //kdDebug(5150) << "Entry " << parsedLine[0] << " val=" << *it << endl;

  if ( !(*it).isEmpty() ) {  // a real value was set
    mSet = true;
    value = *it;
    mValue = stringToValue( value, true );
  }
  else {
    mValue = mDefaultValue;
  }

  mDirty = false;
}

QVariant QGpgMECryptoConfigEntry::stringToValue( const QString& str, bool unescape ) const
{
  bool isString = isStringType();

  if ( isList() ) {
    QValueList<QVariant> lst;
    QStringList items = QStringList::split( ',', str );
    for( QStringList::const_iterator valit = items.begin(); valit != items.end(); ++valit ) {
      QString val = *valit;
      if ( isString ) {
        if ( val.isEmpty() ) {
          lst << QString::null;
          continue;
        }
        else if ( unescape ) {
          if( val[0] != '"' ) // see README.gpgconf
            kdWarning(5150) << "String value should start with '\"' : " << val << endl;
          val = val.mid( 1 );
        }
      }
      lst << QVariant( unescape ? gpgconf_unescape( val ) : val );
    }
    return lst;
  } else { // not a list
    QString val( str );
    if ( isString ) {
      if ( val.isEmpty() )
        return QVariant( QString::null ); // not set  [ok with lists too?]
      else if ( unescape ) {
        Q_ASSERT( val[0] == '"' ); // see README.gpgconf
        val = val.mid( 1 );
      }
    }
    return QVariant( unescape ? gpgconf_unescape( val ) : val );
  }
}

QGpgMECryptoConfigEntry::~QGpgMECryptoConfigEntry()
{
#ifndef NDEBUG
  if ( !s_duringClear && mDirty )
    kdWarning(5150) << "Deleting a QGpgMECryptoConfigEntry that was modified (" << mDescription << ")\n"
                    << "You forgot to call sync() (to commit) or clear() (to discard)" << endl;
#endif
}

bool QGpgMECryptoConfigEntry::isOptional() const
{
  return mFlags & GPGCONF_FLAG_OPTIONAL;
}

bool QGpgMECryptoConfigEntry::isList() const
{
  return mFlags & GPGCONF_FLAG_LIST;
}

bool QGpgMECryptoConfigEntry::isRuntime() const
{
  return mFlags & GPGCONF_FLAG_RUNTIME;
}

bool QGpgMECryptoConfigEntry::isSet() const
{
  return mSet;
}

bool QGpgMECryptoConfigEntry::boolValue() const
{
  Q_ASSERT( mArgType == ArgType_None );
  Q_ASSERT( !isList() );
  return mValue.toBool();
}

QString QGpgMECryptoConfigEntry::stringValue() const
{
  return toString( false );
}

int QGpgMECryptoConfigEntry::intValue() const
{
  Q_ASSERT( mArgType == ArgType_Int );
  Q_ASSERT( !isList() );
  return mValue.toInt();
}

unsigned int QGpgMECryptoConfigEntry::uintValue() const
{
  Q_ASSERT( mArgType == ArgType_UInt );
  Q_ASSERT( !isList() );
  return mValue.toUInt();
}

static KURL parseURL( int mRealArgType, const QString& str )
{
  if ( mRealArgType == 33 ) { // LDAP server
    // The format is HOSTNAME:PORT:USERNAME:PASSWORD:BASE_DN
    QStringList items = QStringList::split( ':', str, true );
    if ( items.count() == 5 ) {
      QStringList::const_iterator it = items.begin();
      KURL url;
      url.setProtocol( "ldap" );
      url.setHost( urlpart_decode( *it++ ) );
      url.setPort( (*it++).toInt() );
      url.setPath( "/" ); // workaround KURL parsing bug
      url.setUser( urlpart_decode( *it++ ) );
      url.setPass( urlpart_decode( *it++ ) );
      url.setQuery( urlpart_decode( *it ) );
      return url;
    } else
      kdWarning(5150) << "parseURL: malformed LDAP server: " << str << endl;
  }
  // other URLs : assume wellformed URL syntax.
  return KURL( str );
}

// The opposite of parseURL
static QString splitURL( int mRealArgType, const KURL& url )
{
  if ( mRealArgType == 33 ) { // LDAP server
    // The format is HOSTNAME:PORT:USERNAME:PASSWORD:BASE_DN
    Q_ASSERT( url.protocol() == "ldap" );
    return urlpart_encode( url.host() ) + ":" +
      QString::number( url.port() ) + ":" +
      urlpart_encode( url.user() ) + ":" +
      urlpart_encode( url.pass() ) + ":" +
      // KURL automatically encoded the query (e.g. for spaces inside it),
      // so decode it before writing it out to gpgconf (issue119)
      urlpart_encode( KURL::decode_string( url.query().mid(1) ) );
  }
  return url.path();
}

KURL QGpgMECryptoConfigEntry::urlValue() const
{
  Q_ASSERT( mArgType == ArgType_Path || mArgType == ArgType_URL || mArgType == ArgType_LDAPURL );
  Q_ASSERT( !isList() );
  QString str = mValue.toString();
  if ( mArgType == ArgType_Path )
  {
    KURL url;
    url.setPath( str );
    return url;
  }
  return parseURL( mRealArgType, str );
}

unsigned int QGpgMECryptoConfigEntry::numberOfTimesSet() const
{
  Q_ASSERT( mArgType == ArgType_None );
  Q_ASSERT( isList() );
  return mValue.toUInt();
}

QStringList QGpgMECryptoConfigEntry::stringValueList() const
{
  Q_ASSERT( isStringType() );
  Q_ASSERT( isList() );
  return mValue.toStringList();
}

QValueList<int> QGpgMECryptoConfigEntry::intValueList() const
{
  Q_ASSERT( mArgType == ArgType_Int );
  Q_ASSERT( isList() );
  QValueList<int> ret;
  QValueList<QVariant> lst = mValue.toList();
  for( QValueList<QVariant>::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
    ret.append( (*it).toInt() );
  }
  return ret;
}

QValueList<unsigned int> QGpgMECryptoConfigEntry::uintValueList() const
{
  Q_ASSERT( mArgType == ArgType_UInt );
  Q_ASSERT( isList() );
  QValueList<unsigned int> ret;
  QValueList<QVariant> lst = mValue.toList();
  for( QValueList<QVariant>::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
    ret.append( (*it).toUInt() );
  }
  return ret;
}

KURL::List QGpgMECryptoConfigEntry::urlValueList() const
{
  Q_ASSERT( mArgType == ArgType_Path || mArgType == ArgType_URL || mArgType == ArgType_LDAPURL );
  Q_ASSERT( isList() );
  QStringList lst = mValue.toStringList();

  KURL::List ret;
  for( QStringList::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( mArgType == ArgType_Path ) {
      KURL url;
      url.setPath( *it );
      ret << url;
    } else {
      ret << parseURL( mRealArgType, *it );
    }
  }
  return ret;
}

void QGpgMECryptoConfigEntry::resetToDefault()
{
  mSet = false;
  mDirty = true;
  if ( mFlags & GPGCONF_FLAG_DEFAULT )
    mValue = mDefaultValue;
  else if ( mArgType == ArgType_None )
    mValue = false;
}

void QGpgMECryptoConfigEntry::setBoolValue( bool b )
{
  Q_ASSERT( mArgType == ArgType_None );
  Q_ASSERT( !isList() );
  // A "no arg" option is either set or not set.
  // Being set means mSet==true + mValue==true, being unset means resetToDefault(), i.e. both false
  mValue = b;
  mSet = b;
  mDirty = true;
}

void QGpgMECryptoConfigEntry::setStringValue( const QString& str )
{
  mValue = stringToValue( str, false );
  // When setting a string to empty (and there's no default), we need to act like resetToDefault
  // Otherwise we try e.g. "ocsp-responder:0:" and gpgconf answers:
  // "gpgconf: argument required for option ocsp-responder"
  if ( str.isEmpty() && !isOptional() )
    mSet = false;
  else
    mSet = true;
  mDirty = true;
}

void QGpgMECryptoConfigEntry::setIntValue( int i )
{
  Q_ASSERT( mArgType == ArgType_Int );
  Q_ASSERT( !isList() );
  mValue = i;
  mSet = true;
  mDirty = true;
}

void QGpgMECryptoConfigEntry::setUIntValue( unsigned int i )
{
  mValue = i;
  mSet = true;
  mDirty = true;
}

void QGpgMECryptoConfigEntry::setURLValue( const KURL& url )
{
  QString str = splitURL( mRealArgType, url );
  if ( str.isEmpty() && !isOptional() )
    mSet = false;
  else
    mSet = true;
  mValue = str;
  mDirty = true;
}

void QGpgMECryptoConfigEntry::setNumberOfTimesSet( unsigned int i )
{
  Q_ASSERT( mArgType == ArgType_None );
  Q_ASSERT( isList() );
  setUIntValue( i );
}

void QGpgMECryptoConfigEntry::setStringValueList( const QStringList& lst )
{
  mValue = lst;
  if ( lst.isEmpty() && !isOptional() )
    mSet = false;
  else
    mSet = true;
  mDirty = true;
}

void QGpgMECryptoConfigEntry::setIntValueList( const QValueList<int>& lst )
{
  QValueList<QVariant> ret;
  for( QValueList<int>::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
    ret << QVariant( *it );
  }
  mValue = ret;
  if ( ret.isEmpty() && !isOptional() )
    mSet = false;
  else
    mSet = true;
  mDirty = true;
}

void QGpgMECryptoConfigEntry::setUIntValueList( const QValueList<unsigned int>& lst )
{
  QValueList<QVariant> ret;
  for( QValueList<unsigned int>::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
    ret << QVariant( *it );
  }
  if ( ret.isEmpty() && !isOptional() )
    mSet = false;
  else
    mSet = true;
  mValue = ret;
  mDirty = true;
}

void QGpgMECryptoConfigEntry::setURLValueList( const KURL::List& urls )
{
  QStringList lst;
  for( KURL::List::const_iterator it = urls.begin(); it != urls.end(); ++it ) {
    lst << splitURL( mRealArgType, *it );
  }
  mValue = lst;
  if ( lst.isEmpty() && !isOptional() )
    mSet = false;
  else
    mSet = true;
  mDirty = true;
}

QString QGpgMECryptoConfigEntry::toString( bool escape ) const
{
  // Basically the opposite of stringToValue
  if ( isStringType() ) {
    if ( mValue.isNull() )
      return QString::null;
    else if ( isList() ) { // string list
      QStringList lst = mValue.toStringList();
      if ( escape ) {
        for( QStringList::iterator it = lst.begin(); it != lst.end(); ++it ) {
          if ( !(*it).isNull() )
            *it = gpgconf_escape( *it ).prepend( "\"" );
        }
      }
      QString res = lst.join( "," );
      kdDebug(5150) << "toString: " << res << endl;
      return res;
    } else { // normal string
      QString res = mValue.toString();
      if ( escape )
        res = gpgconf_escape( res ).prepend( "\"" );
      return res;
    }
  }
  if ( !isList() ) // non-list non-string
  {
    if ( mArgType == ArgType_None ) {
      return mValue.toBool() ? QString::fromLatin1( "1" ) : QString::null;
    } else { // some int
      Q_ASSERT( mArgType == ArgType_Int || mArgType == ArgType_UInt );
      return mValue.toString(); // int to string conversion
    }
  }

  // Lists (of other types than strings)
  if ( mArgType == ArgType_None )
    return QString::number( numberOfTimesSet() );
  QStringList ret;
  QValueList<QVariant> lst = mValue.toList();
  for( QValueList<QVariant>::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
      ret << (*it).toString(); // QVariant does the conversion
  }
  return ret.join( "," );
}

QString QGpgMECryptoConfigEntry::outputString() const
{
  Q_ASSERT( mSet );
  return toString( true );
}

bool QGpgMECryptoConfigEntry::isStringType() const
{
  return ( mArgType == Kleo::CryptoConfigEntry::ArgType_String
           || mArgType == Kleo::CryptoConfigEntry::ArgType_Path
           || mArgType == Kleo::CryptoConfigEntry::ArgType_URL
           || mArgType == Kleo::CryptoConfigEntry::ArgType_LDAPURL );
}

void QGpgMECryptoConfigEntry::setDirty( bool b )
{
  mDirty = b;
}

#include "qgpgmecryptoconfig.moc"
