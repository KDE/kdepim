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

#include <QList>
#include <QByteArray>
#include <kdebug.h>
#include <kprocess.h>
#include <errno.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kshell.h>
#include <KStandardDirs>

#include <gpgme++/engineinfo.h>
#include <gpgme++/global.h>

#include <cassert>
#include <ktemporaryfile.h>
#include <QFile>
#include <cstdlib>
#include <iterator>

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

QString QGpgMECryptoConfig::gpgConfPath()
{
    const GpgME::EngineInfo info = GpgME::engineInfo( GpgME::GpgConfEngine );
    return info.fileName() ? QFile::decodeName( info.fileName() ) : KStandardDirs::findExe( QLatin1String("gpgconf") );
}

QGpgMECryptoConfig::QGpgMECryptoConfig()
 :  mParsed( false )
{
}

QGpgMECryptoConfig::~QGpgMECryptoConfig()
{
    clear();
}

void QGpgMECryptoConfig::runGpgConf( bool showErrors )
{
  // Run gpgconf --list-components to make the list of components
  KProcess process;

  process << gpgConfPath();
  process << QLatin1String("--list-components");


  connect( &process, SIGNAL(readyReadStandardOutput()),
           this, SLOT(slotCollectStdOut()) );

  // run the process:
  int rc = 0;
  process.setOutputChannelMode( KProcess::OnlyStdoutChannel );
  process.start();
  if ( !process.waitForFinished() )
    rc = -2;
  else if ( process.exitStatus() == QProcess::NormalExit )
    rc = process.exitCode();
  else
    rc = -1;

  // handle errors, if any (and if requested)
  if ( showErrors && rc != 0 ) {
    QString reason;
    if ( rc == -1 )
        reason = i18n( "program terminated unexpectedly" );
    else if ( rc == -2 )
        reason = i18n( "program not found or cannot be started" );
    else
      reason = QString::fromLocal8Bit( strerror(rc) ); // XXX errno as an exit code?
    QString wmsg = i18n("<qt>Failed to execute gpgconf:<p>%1</p></qt>", reason);
    kWarning(5150) << wmsg; // to see it from test_cryptoconfig.cpp
    KMessageBox::error(0, wmsg);
  }
  mParsed = true;
}

void QGpgMECryptoConfig::slotCollectStdOut()
{
  assert( qobject_cast<KProcess*>( QObject::sender() ) );
  KProcess * const proc = static_cast<KProcess*>( QObject::sender() );
  while( proc->canReadLine() ) {
    QString line = QString::fromUtf8( proc->readLine() );
    if ( line.endsWith( QLatin1Char('\n') ) )
      line.chop( 1 );
    if ( line.endsWith( QLatin1Char('\r') ) )
      line.chop( 1 );
    //kDebug(5150) <<"GOT LINE:" << line;
    // Format: NAME:DESCRIPTION
    const QStringList lst = line.split( QLatin1Char(':') );
    if ( lst.count() >= 2 ) {
        const std::pair<QString,QGpgMECryptoConfigComponent*> pair( lst[0], new QGpgMECryptoConfigComponent( this, lst[0], lst[1] ) );
        mComponentsNaturalOrder.push_back( pair );
        mComponentsByName[pair.first] = pair.second;
    } else {
      kWarning(5150) <<"Parse error on gpgconf --list-components output:" << line;
    }
  }
}

namespace {
    struct Select1St {
        template <typename U, typename V>
        const U & operator()( const std::pair<U,V> & p ) const { return p.first; }
        template <typename U, typename V>
        const U & operator()( const QPair<U,V> & p ) const { return p.first; }
    };
}

QStringList QGpgMECryptoConfig::componentList() const
{
  if ( !mParsed )
    const_cast<QGpgMECryptoConfig*>( this )->runGpgConf( true );
  QStringList result;
  std::transform( mComponentsNaturalOrder.begin(), mComponentsNaturalOrder.end(),
                  std::back_inserter( result ), Select1St() );
  return result;
}

Kleo::CryptoConfigComponent* QGpgMECryptoConfig::component( const QString& name ) const
{
  if ( !mParsed )
    const_cast<QGpgMECryptoConfig*>( this )->runGpgConf( false );
  return mComponentsByName.value( name );
}

void QGpgMECryptoConfig::sync( bool runtime )
{
  Q_FOREACH (QGpgMECryptoConfigComponent *it, mComponentsByName)
      it->sync(runtime);
}

void QGpgMECryptoConfig::clear()
{
  s_duringClear = true;
  mComponentsNaturalOrder.clear();
  qDeleteAll(mComponentsByName);
  mComponentsByName.clear();
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
  mGroupsNaturalOrder.clear();
  qDeleteAll(mGroupsByName);
  mGroupsByName.clear();
}

void QGpgMECryptoConfigComponent::runGpgConf()
{
  const QString gpgconf = QGpgMECryptoConfig::gpgConfPath();
  if ( gpgconf.isEmpty() ) {
      kWarning(5150) << "Can't get path to gpgconf executable...";
      return;
  }

  // Run gpgconf --list-options <component>, and create all groups and entries for that component
  KProcess proc;
  proc << gpgconf;
  proc << QLatin1String("--list-options");
  proc << mName;

  //kDebug(5150) <<"Running gpgconf --list-options" << mName;

  connect( &proc, SIGNAL(readyReadStandardOutput()),
           this, SLOT(slotCollectStdOut()) );
  mCurrentGroup = 0;

  // run the process:
  int rc = 0;
  proc.setOutputChannelMode( KProcess::OnlyStdoutChannel );
  proc.start();
  if ( !proc.waitForFinished() )
    rc = -2;
  else if ( proc.exitStatus() == QProcess::NormalExit )
    rc = proc.exitCode();
  else
    rc = -1;

  if( rc != 0 ) // can happen when using the wrong version of gpg...
    kWarning(5150) <<"Running 'gpgconf --list-options" << mName <<"' failed." << strerror( rc ) <<", but try that command to see the real output";
  else {
    if ( mCurrentGroup && !mCurrentGroup->mEntriesNaturalOrder.empty() ) { // only add non-empty groups
      mGroupsByName.insert( mCurrentGroupName, mCurrentGroup );
      mGroupsNaturalOrder.push_back( std::make_pair( mCurrentGroupName, mCurrentGroup ) );
    }
  }
}

void QGpgMECryptoConfigComponent::slotCollectStdOut()
{
  assert( qobject_cast<KProcess*>( QObject::sender() ) );
  KProcess * const proc = static_cast<KProcess*>( QObject::sender() );
  while( proc->canReadLine() ) {
    QString line = QString::fromUtf8( proc->readLine() );
    if ( line.endsWith( QLatin1Char('\n') ) )
      line.chop( 1 );
    if ( line.endsWith( QLatin1Char('\r') ) )
      line.chop( 1 );
    //kDebug(5150) <<"GOT LINE:" << line;
    // Format: NAME:FLAGS:LEVEL:DESCRIPTION:TYPE:ALT-TYPE:ARGNAME:DEFAULT:ARGDEF:VALUE
    const QStringList lst = line.split( QLatin1Char(':') );
    if ( lst.count() >= 10 ) {
      const int flags = lst[1].toInt();
      const int level = lst[2].toInt();
      if ( level > 2 ) // invisible or internal -> skip it;
        continue;
      if ( flags & GPGCONF_FLAG_GROUP ) {
        if ( mCurrentGroup && !mCurrentGroup->mEntriesNaturalOrder.empty() ) { // only add non-empty groups
          mGroupsByName.insert( mCurrentGroupName, mCurrentGroup );
          mGroupsNaturalOrder.push_back( std::make_pair( mCurrentGroupName, mCurrentGroup ) );
        }
        //else
        //  kDebug(5150) <<"Discarding empty group" << mCurrentGroupName;
        mCurrentGroup = new QGpgMECryptoConfigGroup( this, lst[0], lst[3], level );
        mCurrentGroupName = lst[0];
      } else {
        // normal entry
        if ( !mCurrentGroup ) {  // first toplevel entry -> create toplevel group
          mCurrentGroup = new QGpgMECryptoConfigGroup( this, QLatin1String("<nogroup>"), QString(), 0 );
          mCurrentGroupName = QLatin1String("<nogroup>");
        }
        const QString & name = lst[0];
        QGpgMECryptoConfigEntry * value = new QGpgMECryptoConfigEntry( mCurrentGroup, lst );
        mCurrentGroup->mEntriesByName.insert( name, value );
        mCurrentGroup->mEntriesNaturalOrder.push_back( std::make_pair( name, value ) );
      }
    } else {
      // This happens on lines like
      // dirmngr[31465]: error opening `/home/dfaure/.gnupg/dirmngr_ldapservers.conf': No such file or directory
      // so let's not bother the user with it.
      //kWarning(5150) <<"Parse error on gpgconf --list-options output:" << line;
    }
  }
}

QStringList QGpgMECryptoConfigComponent::groupList() const
{
  QStringList result;
  std::transform( mGroupsNaturalOrder.begin(), mGroupsNaturalOrder.end(),
                  std::back_inserter( result ), Select1St() );
  return result;
}

Kleo::CryptoConfigGroup* QGpgMECryptoConfigComponent::group(const QString& name ) const
{
  return mGroupsByName.value( name );
}

void QGpgMECryptoConfigComponent::sync( bool runtime )
{
  KTemporaryFile tmpFile;
  tmpFile.open();

  QList<QGpgMECryptoConfigEntry *> dirtyEntries;

  // Collect all dirty entries
  const QList<QString> keylist = mGroupsByName.uniqueKeys();
  Q_FOREACH (const QString & key, keylist) {
    const QHash<QString,QGpgMECryptoConfigEntry*> entry = mGroupsByName[key]->mEntriesByName;
    const QList<QString> keylistentry = entry.uniqueKeys();
    Q_FOREACH (const QString & keyentry, keylistentry) {
      if(entry[keyentry]->isDirty())
      {
       // OK, we can set it.currentKey() to it.current()->outputString()
        QString line = keyentry;
        if ( entry[keyentry]->isSet() ) { // set option
          line += QLatin1String(":0:");
          line += entry[keyentry]->outputString();
        } else {                       // unset option
          line += QLatin1String(":16:");
        }
#ifdef Q_OS_WIN
        line += QLatin1Char('\r');
#endif
        line += QLatin1Char('\n');
        const QByteArray line8bit = line.toUtf8(); // encode with utf8, and K3ProcIO uses utf8 when reading.
        tmpFile.write( line8bit );
        dirtyEntries.append( entry[keyentry] );

      }
    }
  }

  tmpFile.flush();
  if ( dirtyEntries.isEmpty() )
    return;

  // Call gpgconf --change-options <component>
  const QString gpgconf = QGpgMECryptoConfig::gpgConfPath();
  QString commandLine = gpgconf.isEmpty()
      ? QString::fromLatin1( "gpgconf" )
      : KShell::quoteArg( gpgconf ) ;
  if ( runtime )
    commandLine += QLatin1String(" --runtime");
  commandLine += QLatin1String(" --change-options ");
  commandLine += KShell::quoteArg( mName );
  commandLine += QLatin1String(" < ");
  commandLine += KShell::quoteArg( tmpFile.fileName() );

  //kDebug(5150) << commandLine;
  //system( QCString( "cat " ) + tmpFile.name().toLatin1() ); // DEBUG

  KProcess proc;
  proc.setShellCommand( commandLine );

  // run the process:
  int rc = proc.execute();

  if ( rc == -2 )
  {
    QString wmsg = i18n( "Could not start gpgconf.\nCheck that gpgconf is in the PATH and that it can be started." );
    kWarning(5150) << wmsg;
    KMessageBox::error(0, wmsg);
  }
  else if( rc != 0 ) // Happens due to bugs in gpgconf (e.g. issues 104/115)
  {
    QString wmsg = i18n( "Error from gpgconf while saving configuration: %1", QString::fromLocal8Bit( strerror( rc ) ) );
    kWarning(5150) <<":" << strerror( rc );
    KMessageBox::error(0, wmsg);
  }
  else
  {
    QList<QGpgMECryptoConfigEntry *>::const_iterator it = dirtyEntries.constBegin();
    for( ; it != dirtyEntries.constEnd(); ++it ) {
      (*it)->setDirty( false );
    }
  }
}

////

QGpgMECryptoConfigGroup::QGpgMECryptoConfigGroup( QGpgMECryptoConfigComponent * comp, const QString & name, const QString& description, int level )
  :
    mComponent( comp ),
    mName( name ),
    mDescription( description ),
    mLevel( static_cast<Kleo::CryptoConfigEntry::Level>( level ) )
{
}

QGpgMECryptoConfigGroup::~QGpgMECryptoConfigGroup()
{
  mEntriesNaturalOrder.clear();
  qDeleteAll(mEntriesByName);
  mEntriesByName.clear();
}

QStringList QGpgMECryptoConfigGroup::entryList() const
{
  QStringList result;
  std::transform( mEntriesNaturalOrder.begin(), mEntriesNaturalOrder.end(),
                  std::back_inserter( result ), Select1St() );
  return result;
}

Kleo::CryptoConfigEntry* QGpgMECryptoConfigGroup::entry( const QString& name ) const
{
  return mEntriesByName.value( name );
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
  QString enc = QLatin1String(KUrl::toPercentEncoding( str )); // and convert to utf8 first (to get %12%34 for one special char)
  // Also encode commas, for lists.
  enc.replace( QLatin1Char(','), QLatin1String("%2c") );
  return enc;
}

static QString urlpart_encode( const QString& str )
{
  QString enc( str );
  enc.replace( QLatin1Char('%'), QLatin1String("%25") ); // first!
  enc.replace( QLatin1Char(':'), QLatin1String("%3a") );
  //kDebug(5150) <<"  urlpart_encode:" << str <<" ->" << enc;
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

QGpgMECryptoConfigEntry::QGpgMECryptoConfigEntry( QGpgMECryptoConfigGroup * group, const QStringList& parsedLine )
    : mGroup( group )
{
  // Format: NAME:FLAGS:LEVEL:DESCRIPTION:TYPE:ALT-TYPE:ARGNAME:DEFAULT:ARGDEF:VALUE
  assert( parsedLine.count() >= 10 ); // called checked for it already
  QStringList::const_iterator it = parsedLine.constBegin();
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
    kWarning(5150) <<"Unsupported datatype:" << parsedLine[4] <<" :" << *it <<" for" << parsedLine[0];
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
  //kDebug(5150) <<"Entry" << parsedLine[0] <<" val=" << *it;

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
  const bool isString = isStringType();

  if ( isList() ) {
    if ( argType() == ArgType_None ) {
        bool ok = true;
        const QVariant v = str.isEmpty() ? 0U : str.toUInt( &ok ) ;
        if ( !ok )
            kWarning(5150) << "list-of-none should have an unsigned int as value:" << str;
        return v;
    }
    QList<QVariant> lst;
    QStringList items = str.split( QLatin1Char(','), QString::SkipEmptyParts );
    for( QStringList::const_iterator valit = items.constBegin(); valit != items.constEnd(); ++valit ) {
      QString val = *valit;
      if ( isString ) {
        if ( val.isEmpty() ) {
          lst << QVariant( QString() );
          continue;
        }
        else if ( unescape ) {
          if( val[0] != QLatin1Char('"') ) // see README.gpgconf
            kWarning(5150) <<"String value should start with '\"' :" << val;
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
        if( val[0] != QLatin1Char('"') ) // see README.gpgconf
          kWarning(5150) <<"String value should start with '\"' :" << val;
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
    kWarning(5150) <<"Deleting a QGpgMECryptoConfigEntry that was modified (" << mDescription <<")"
                    << "You forgot to call sync() (to commit) or clear() (to discard)";
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
    QStringList items = str.split( QLatin1Char(':') );
    if ( items.count() == 5 ) {
      QStringList::const_iterator it = items.constBegin();
      KUrl url;
      url.setProtocol( QLatin1String("ldap") );
      url.setHost( urlpart_decode( *it++ ) );

      bool ok;
      const int port = (*it++).toInt( &ok );
      if ( ok )
          url.setPort( port );
      else if ( !it->isEmpty() )
          kWarning(5150) <<"parseURL: malformed LDAP server port, ignoring: \"" << *it << "\"";

      url.setPath( QLatin1String("/") ); // workaround KUrl parsing bug
      url.setUserName( urlpart_decode( *it++ ) );
      url.setPassword( urlpart_decode( *it++ ) );
      url.setQuery( urlpart_decode( *it ) );
      return url;
    } else
      kWarning(5150) <<"parseURL: malformed LDAP server:" << str;
  }
  // other URLs : assume wellformed URL syntax.
  return KUrl( str );
}

// The opposite of parseURL
static QString splitURL( int mRealArgType, const KUrl& url )
{
  if ( mRealArgType == 33 ) { // LDAP server
    // The format is HOSTNAME:PORT:USERNAME:PASSWORD:BASE_DN
    Q_ASSERT( url.protocol() == QLatin1String("ldap") );
    return urlpart_encode( url.host() ) + QLatin1Char(':') +
      ( url.port() != -1 ? QString::number( url.port() ) : QString() ) + QLatin1Char(':') + // -1 is used for default ports, omit
      urlpart_encode( url.user() ) + QLatin1Char(':') +
      urlpart_encode( url.pass() ) + QLatin1Char(':') +
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

std::vector<int> QGpgMECryptoConfigEntry::intValueList() const
{
  Q_ASSERT( mArgType == ArgType_Int );
  Q_ASSERT( isList() );
  std::vector<int> ret;
  QList<QVariant> lst = mValue.toList();
  ret.reserve( lst.size() );
  for( QList<QVariant>::const_iterator it = lst.constBegin(); it != lst.constEnd(); ++it ) {
    ret.push_back( (*it).toInt() );
  }
  return ret;
}

std::vector<unsigned int> QGpgMECryptoConfigEntry::uintValueList() const
{
  Q_ASSERT( mArgType == ArgType_UInt );
  Q_ASSERT( isList() );
  std::vector<unsigned int> ret;
  QList<QVariant> lst = mValue.toList();
  ret.reserve( lst.size() );
  for( QList<QVariant>::const_iterator it = lst.constBegin(); it != lst.constEnd(); ++it ) {
    ret.push_back( (*it).toUInt() );
  }
  return ret;
}

KUrl::List QGpgMECryptoConfigEntry::urlValueList() const
{
  Q_ASSERT( mArgType == ArgType_Path || mArgType == ArgType_URL || mArgType == ArgType_LDAPURL );
  Q_ASSERT( isList() );
  QStringList lst = mValue.toStringList();

  KUrl::List ret;
  for( QStringList::const_iterator it = lst.constBegin(); it != lst.constEnd(); ++it ) {
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
  else if ( mArgType == ArgType_None ) {
    if ( isList() )
      mValue = 0U;
    else
      mValue = false;
  }
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
  mValue = i;
  mSet = i > 0;
  mDirty = true;
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

void QGpgMECryptoConfigEntry::setIntValueList( const std::vector<int>& lst )
{
  QList<QVariant> ret;
  for( std::vector<int>::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
    ret << QVariant( *it );
  }
  mValue = ret;
  if ( ret.isEmpty() && !isOptional() )
    mSet = false;
  else
    mSet = true;
  mDirty = true;
}

void QGpgMECryptoConfigEntry::setUIntValueList( const std::vector<unsigned int>& lst )
{
  QList<QVariant> ret;
  for( std::vector<unsigned int>::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
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
  for( KUrl::List::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it ) {
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
            *it = gpgconf_escape( *it ).prepend( QLatin1String("\"") );
        }
      }
      const QString res = lst.join( QLatin1String(",") );
      //kDebug(5150) <<"toString:" << res;
      return res;
    } else { // normal string
      QString res = mValue.toString();
      if ( escape )
        res = gpgconf_escape( res ).prepend( QLatin1String("\"") );
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
  for( QList<QVariant>::const_iterator it = lst.constBegin(); it != lst.constEnd(); ++it ) {
      ret << (*it).toString(); // QVariant does the conversion
  }
  return ret.join( QLatin1String(",") );
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

