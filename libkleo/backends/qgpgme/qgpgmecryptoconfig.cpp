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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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
//Added by qt3to4:
#include <QList>
#include <QByteArray>
#include <kdebug.h>
#include <k3procio.h>
#include <kprocess.h>
#include <errno.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kshell.h>

#include <assert.h>
#include <ktemporaryfile.h>
#include <QFile>
#include <stdlib.h>
#include <QTextCodec>

// Just for the Q_ASSERT in the dtor. Not thread-safe, but who would
// have 2 threads talking to gpgconf anyway? :)
static bool s_duringClear = false;

static const int GPGCONF_FLAG_GROUP = 1;
static const int GPGCONF_FLAG_OPTIONAL = 2;
static const int GPGCONF_FLAG_LIST = 4;
static const int GPGCONF_FLAG_RUNTIME = 8;
static const int GPGCONF_FLAG_DEFAULT = 16; // fixed default value available
static const int GPGCONF_FLAG_DEFAULT_DESC = 32; // runtime default value available
static const int GPGCONF_FLAG_NOARG_DESC = 64; // option with optional arg; special meaning if no arg set
static const int GPGCONF_FLAG_NO_CHANGE = 128; // readonly
// Change size of mFlags bitfield if adding new values here

QGpgMECryptoConfig::QGpgMECryptoConfig()
 :  mParsed( false )
{
}

QGpgMECryptoConfig::~QGpgMECryptoConfig()
{
}

void QGpgMECryptoConfig::runGpgConf( bool showErrors )
{
  // Run gpgconf --list-components to make the list of components

  mProcess =new KProcess;
  *mProcess << "gpgconf"; // must be in the PATH
  *mProcess << "--list-components";


  QObject::connect( mProcess, SIGNAL(readyReadStandardOutput()),
                    this, SLOT( slotCollectStdOut() ) );

  // run the process:
  int rc = 0;
  mProcess->setOutputChannelMode(KProcess::MergedChannels);
  mProcess->start();
  if ( !mProcess->waitForFinished() )
    rc = -2;
  else
    rc = ( mProcess->exitStatus () == QProcess::NormalExit ) ? mProcess->exitCode() : -1 ;

  // handle errors, if any (and if requested)
  if ( showErrors && rc != 0 ) {
    QString reason;
    if ( rc == -1 )
        reason = i18n( "program terminated unexpectedly" );
    else if ( rc == -2 )
        reason = i18n( "program not found or cannot be started" );
    else
        reason = strerror(rc); // XXX errno as an exit code?
    QString wmsg = i18n("<qt>Failed to execute gpgconf:<p>%1</p></qt>", reason);
    kWarning(5150) << wmsg << endl; // to see it from test_cryptoconfig.cpp
    KMessageBox::error(0, wmsg);
  }
  mParsed = true;
}

void QGpgMECryptoConfig::slotCollectStdOut()
{
  QString line;
  int result;
  while( mProcess->canReadLine() ) {
     line = QString::fromLocal8Bit(mProcess->readLine());
    kDebug(5150) << "GOT LINE:" << line << endl;
    // Format: NAME:DESCRIPTION
    QStringList lst = line.split( ':' );
    if ( lst.count() >= 2 ) {
      mComponents.insert( lst[0], new QGpgMECryptoConfigComponent( this, lst[0], lst[1] ) );
    } else {
      kWarning(5150) << "Parse error on gpgconf --list-components output: " << line << endl;
    }
  }
}

QStringList QGpgMECryptoConfig::componentList() const
{
  if ( !mParsed )
    const_cast<QGpgMECryptoConfig*>( this )->runGpgConf( true );
  QStringList names;
  QList<QString> keylist = mComponents.uniqueKeys();
  foreach (QString key, keylist) {
	names << key;
  }
  return names;
}

Kleo::CryptoConfigComponent* QGpgMECryptoConfig::component( const QString& name ) const
{
  if ( !mParsed )
    const_cast<QGpgMECryptoConfig*>( this )->runGpgConf( false );
  return mComponents.value( name );
}

void QGpgMECryptoConfig::sync( bool runtime )
{
  foreach (QGpgMECryptoConfigComponent *it, mComponents){
  	it->sync(runtime);
  }
}

void QGpgMECryptoConfig::clear()
{
  s_duringClear = true;
  qDeleteAll(mComponents);
  mComponents.clear();
  s_duringClear = false;
  mParsed = false; // next call to componentList/component will need to run gpgconf again
}

////

QGpgMECryptoConfigComponent::QGpgMECryptoConfigComponent( QGpgMECryptoConfig*, const QString& name, const QString& description )
  : mName( name ), mDescription( description )
{
  runGpgConf();
}

QGpgMECryptoConfigComponent::~QGpgMECryptoConfigComponent()
{
  qDeleteAll(mGroups);
  mGroups.clear();
}

void QGpgMECryptoConfigComponent::runGpgConf()
{
  // Run gpgconf --list-options <component>, and create all groups and entries for that component

  K3ProcIO proc( QTextCodec::codecForName( "utf8" ) );
  proc << "gpgconf"; // must be in the PATH
  proc << "--list-options";
  proc << mName;

  //kDebug(5150) << "Running gpgconf --list-options " << mName << endl;

  QObject::connect( &proc, SIGNAL( readReady(K3ProcIO*) ),
                    this, SLOT( slotCollectStdOut(K3ProcIO*) ) );
  mCurrentGroup = 0;

  // run the process:
  int rc = 0;
  if ( !proc.start( K3Process::Block ) )
    rc = -2;
  else
    rc = ( proc.normalExit() ) ? proc.exitStatus() : -1 ;

  if( rc != 0 ) // can happen when using the wrong version of gpg...
    kWarning(5150) << "Running 'gpgconf --list-options " << mName << "' failed. " << strerror( rc ) << ", but try that command to see the real output" << endl;
  else {
    if ( mCurrentGroup && !mCurrentGroup->mEntries.isEmpty() ) // only add non-empty groups
      mGroups.insert( mCurrentGroupName, mCurrentGroup );
  }
}

void QGpgMECryptoConfigComponent::slotCollectStdOut( K3ProcIO* proc )
{
  QString line;
  int result;
  while( ( result = proc->readln(line) ) != -1 ) {
    //kDebug(5150) << "GOT LINE:" << line << endl;
    // Format: NAME:FLAGS:LEVEL:DESCRIPTION:TYPE:ALT-TYPE:ARGNAME:DEFAULT:ARGDEF:VALUE
    const QStringList lst = line.split( ':' );
    if ( lst.count() >= 10 ) {
      const int flags = lst[1].toInt();
      const int level = lst[2].toInt();
      if ( level > 2 ) // invisible or internal -> skip it;
        continue;
      if ( flags & GPGCONF_FLAG_GROUP ) {
        if ( mCurrentGroup && !mCurrentGroup->mEntries.isEmpty() ) // only add non-empty groups
          mGroups.insert( mCurrentGroupName, mCurrentGroup );
        //else
        //  kDebug(5150) << "Discarding empty group " << mCurrentGroupName << endl;
        mCurrentGroup = new QGpgMECryptoConfigGroup( lst[0], lst[3], level );
        mCurrentGroupName = lst[0];
      } else {
        // normal entry
        if ( !mCurrentGroup ) {  // first toplevel entry -> create toplevel group
          mCurrentGroup = new QGpgMECryptoConfigGroup( "<nogroup>", QString(), 0 );
          mCurrentGroupName = "<nogroup>";
        }
        mCurrentGroup->mEntries.insert( lst[0], new QGpgMECryptoConfigEntry( lst ) );
      }
    } else {
      // This happens on lines like
      // dirmngr[31465]: error opening `/home/dfaure/.gnupg/dirmngr_ldapservers.conf': No such file or directory
      // so let's not bother the user with it.
      //kWarning(5150) << "Parse error on gpgconf --list-options output: " << line << endl;
    }
  }
}

QStringList QGpgMECryptoConfigComponent::groupList() const
{
  QStringList names;
  QList<QString> keylist = mGroups.uniqueKeys();
  foreach (QString key, keylist) {
        names << key;
  }
  return names;
}

Kleo::CryptoConfigGroup* QGpgMECryptoConfigComponent::group(const QString& name ) const
{
  return mGroups.value( name );
}

void QGpgMECryptoConfigComponent::sync( bool runtime )
{
  KTemporaryFile tmpFile;
  tmpFile.open();

  QList<QGpgMECryptoConfigEntry *> dirtyEntries;

  // Collect all dirty entries
  QList<QString> keylist = mGroups.uniqueKeys();
  foreach (QString key, keylist) {
    QHash<QString,QGpgMECryptoConfigEntry*> entry = mGroups[key]->mEntries;
    QList<QString> keylistentry = entry.uniqueKeys();
    foreach (QString keyentry, keylistentry) {
      if(entry[keyentry]->isDirty())
      {
       // OK, we can set it.currentKey() to it.current()->outputString()
        QString line = keyentry;
        if ( entry[keyentry]->isSet() ) { // set option
          line += ":0:";
          line += entry[keyentry]->outputString();
        } else {                       // unset option
          line += ":16:";
        }
        line += '\n';
        QByteArray line8bit = line.toUtf8(); // encode with utf8, and K3ProcIO uses utf8 when reading.
        tmpFile.write( line8bit.data(), line8bit.size()-1 /*no 0*/ );
        dirtyEntries.append( entry[keyentry] );

      }
    }
  }

  tmpFile.flush();
  if ( dirtyEntries.isEmpty() )
    return;

  // Call gpgconf --change-options <component>
  QString commandLine = "gpgconf";
  if ( runtime )
    commandLine += " --runtime";
  commandLine += " --change-options ";
  commandLine += KShell::quoteArg( mName );
  commandLine += " < ";
  commandLine += KShell::quoteArg( tmpFile.fileName() );

  //kDebug(5150) << commandLine << endl;
  //system( QCString( "cat " ) + tmpFile.name().toLatin1() ); // DEBUG

  KProcess proc;
  proc.setShellCommand( commandLine );

  // run the process:
  int rc = proc.execute();

  if ( rc == -2 )
  {
    QString wmsg = i18n( "Could not start gpgconf\nCheck that gpgconf is in the PATH and that it can be started" );
    kWarning(5150) << wmsg << endl;
    KMessageBox::error(0, wmsg);
  }
  else if( rc != 0 ) // Happens due to bugs in gpgconf (e.g. issues 104/115)
  {
    QString wmsg = i18n( "Error from gpgconf while saving configuration: %1", strerror( rc ) );
    kWarning(5150) << k_funcinfo << ":" << strerror( rc ) << endl;
    KMessageBox::error(0, wmsg);
  }
  else
  {
    QList<QGpgMECryptoConfigEntry *>::Iterator it = dirtyEntries.begin();
    for( ; it != dirtyEntries.end(); ++it ) {
      (*it)->setDirty( false );
    }
  }
}

////

QGpgMECryptoConfigGroup::QGpgMECryptoConfigGroup( const QString & name, const QString& description, int level )
  :
    mName( name ),
    mDescription( description ),
    mLevel( static_cast<Kleo::CryptoConfigEntry::Level>( level ) )
{
}

QGpgMECryptoConfigGroup::~QGpgMECryptoConfigGroup()
{
  qDeleteAll(mEntries);
  mEntries.clear();
}

QStringList QGpgMECryptoConfigGroup::entryList() const
{
  QStringList names;
  QList<QString> keylist = mEntries.uniqueKeys();
  foreach (QString key, keylist) {
        names << key;
  }
  return names;
}

Kleo::CryptoConfigEntry* QGpgMECryptoConfigGroup::entry( const QString& name ) const
{
  return mEntries.value( name );
}

////

static QString gpgconf_unescape( const QString& str )
{
  // Looks like it's the same rules as KUrl.
  return KUrl::fromPercentEncoding( str.toLatin1() );
}

static QString gpgconf_escape( const QString& str )
{
  // Escape special chars (including ':' and '%')
  QString enc = KUrl::toPercentEncoding( str ); // and convert to utf8 first (to get %12%34 for one special char)
  // Also encode commas, for lists.
  enc.replace( ',', "%2c" );
  return enc;
}

static QString urlpart_encode( const QString& str )
{
  QString enc( str );
  enc.replace( '%', "%25" ); // first!
  enc.replace( ':', "%3a" );
  //kDebug() << "  urlpart_encode: " << str << " -> " << enc << endl;
  return enc;
}

static QString urlpart_decode( const QString& str )
{
  return KUrl::fromPercentEncoding( str.toLatin1() );
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
  mName = *it++;
  mFlags = (*it++).toInt();
  mLevel = (*it++).toInt();
  mDescription = *it++;
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
    kWarning(5150) << "Unsupported datatype: " << parsedLine[4] << " : " << *it << " for " << parsedLine[0] << endl;
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
  //kDebug(5150) << "Entry " << parsedLine[0] << " val=" << *it << endl;

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
    QList<QVariant> lst;
    QStringList items = str.split( ',', QString::SkipEmptyParts );
    for( QStringList::const_iterator valit = items.begin(); valit != items.end(); ++valit ) {
      QString val = *valit;
      if ( isString ) {
        if ( val.isEmpty() ) {
          lst << QVariant( QString() );
          continue;
        }
        else if ( unescape ) {
          if( val[0] != '"' ) // see README.gpgconf
            kWarning(5150) << "String value should start with '\"' : " << val << endl;
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
        return QVariant( QString() ); // not set  [ok with lists too?]
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
    kWarning(5150) << "Deleting a QGpgMECryptoConfigEntry that was modified (" << mDescription << ")\n"
                    << "You forgot to call sync() (to commit) or clear() (to discard)" << endl;
#endif
}

bool QGpgMECryptoConfigEntry::isOptional() const
{
  return mFlags & GPGCONF_FLAG_OPTIONAL;
}

bool QGpgMECryptoConfigEntry::isReadOnly() const
{
  return mFlags & GPGCONF_FLAG_NO_CHANGE;
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

static KUrl parseURL( int mRealArgType, const QString& str )
{
  if ( mRealArgType == 33 ) { // LDAP server
    // The format is HOSTNAME:PORT:USERNAME:PASSWORD:BASE_DN
    QStringList items = str.split( ':' );
    if ( items.count() == 5 ) {
      QStringList::const_iterator it = items.begin();
      KUrl url;
      url.setProtocol( "ldap" );
      url.setHost( urlpart_decode( *it++ ) );
      url.setPort( (*it++).toInt() );
      url.setPath( "/" ); // workaround KUrl parsing bug
      url.setUser( urlpart_decode( *it++ ) );
      url.setPass( urlpart_decode( *it++ ) );
      url.setQuery( urlpart_decode( *it ) );
      return url;
    } else
      kWarning(5150) << "parseURL: malformed LDAP server: " << str << endl;
  }
  // other URLs : assume wellformed URL syntax.
  return KUrl( str );
}

// The opposite of parseURL
static QString splitURL( int mRealArgType, const KUrl& url )
{
  if ( mRealArgType == 33 ) { // LDAP server
    // The format is HOSTNAME:PORT:USERNAME:PASSWORD:BASE_DN
    Q_ASSERT( url.protocol() == "ldap" );
    return urlpart_encode( url.host() ) + ':' +
      QString::number( url.port() ) + ':' +
      urlpart_encode( url.user() ) + ':' +
      urlpart_encode( url.pass() ) + ':' +
      // KUrl automatically encoded the query (e.g. for spaces inside it),
      // so decode it before writing it out to gpgconf (issue119)
      urlpart_encode( KUrl::fromPercentEncoding( url.query().mid(1).toLatin1() ) );
  }
  return url.path();
}

KUrl QGpgMECryptoConfigEntry::urlValue() const
{
  Q_ASSERT( mArgType == ArgType_Path || mArgType == ArgType_URL || mArgType == ArgType_LDAPURL );
  Q_ASSERT( !isList() );
  QString str = mValue.toString();
  if ( mArgType == ArgType_Path )
  {
    KUrl url;
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

QList<int> QGpgMECryptoConfigEntry::intValueList() const
{
  Q_ASSERT( mArgType == ArgType_Int );
  Q_ASSERT( isList() );
  QList<int> ret;
  QList<QVariant> lst = mValue.toList();
  for( QList<QVariant>::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
    ret.append( (*it).toInt() );
  }
  return ret;
}

QList<unsigned int> QGpgMECryptoConfigEntry::uintValueList() const
{
  Q_ASSERT( mArgType == ArgType_UInt );
  Q_ASSERT( isList() );
  QList<unsigned int> ret;
  QList<QVariant> lst = mValue.toList();
  for( QList<QVariant>::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
    ret.append( (*it).toUInt() );
  }
  return ret;
}

KUrl::List QGpgMECryptoConfigEntry::urlValueList() const
{
  Q_ASSERT( mArgType == ArgType_Path || mArgType == ArgType_URL || mArgType == ArgType_LDAPURL );
  Q_ASSERT( isList() );
  QStringList lst = mValue.toStringList();

  KUrl::List ret;
  for( QStringList::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( mArgType == ArgType_Path ) {
      KUrl url;
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

void QGpgMECryptoConfigEntry::setURLValue( const KUrl& url )
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

void QGpgMECryptoConfigEntry::setIntValueList( const QList<int>& lst )
{
  QList<QVariant> ret;
  for( QList<int>::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
    ret << QVariant( *it );
  }
  mValue = ret;
  if ( ret.isEmpty() && !isOptional() )
    mSet = false;
  else
    mSet = true;
  mDirty = true;
}

void QGpgMECryptoConfigEntry::setUIntValueList( const QList<unsigned int>& lst )
{
  QList<QVariant> ret;
  for( QList<unsigned int>::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
    ret << QVariant( *it );
  }
  if ( ret.isEmpty() && !isOptional() )
    mSet = false;
  else
    mSet = true;
  mValue = ret;
  mDirty = true;
}

void QGpgMECryptoConfigEntry::setURLValueList( const KUrl::List& urls )
{
  QStringList lst;
  for( KUrl::List::const_iterator it = urls.begin(); it != urls.end(); ++it ) {
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
      return QString();
    else if ( isList() ) { // string list
      QStringList lst = mValue.toStringList();
      if ( escape ) {
        for( QStringList::iterator it = lst.begin(); it != lst.end(); ++it ) {
          if ( !(*it).isNull() )
            *it = gpgconf_escape( *it ).prepend( "\"" );
        }
      }
      QString res = lst.join( "," );
      kDebug(5150) << "toString: " << res << endl;
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
      return mValue.toBool() ? QString::fromLatin1( "1" ) : QString();
    } else { // some int
      Q_ASSERT( mArgType == ArgType_Int || mArgType == ArgType_UInt );
      return mValue.toString(); // int to string conversion
    }
  }

  // Lists (of other types than strings)
  if ( mArgType == ArgType_None )
    return QString::number( numberOfTimesSet() );
  QStringList ret;
  QList<QVariant> lst = mValue.toList();
  for( QList<QVariant>::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
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
