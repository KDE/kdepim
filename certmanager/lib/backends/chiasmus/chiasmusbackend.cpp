/*
    chiasmusbackend.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2005 Klarälvdalens Datakonsult AB

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "chiasmusbackend.h"

#include "config_data.h"
#include "obtainkeysjob.h"
#include "chiasmusjob.h"

#include "kleo/cryptoconfig.h"

#include <klocale.h>
#include <kconfig.h>
#include <kshell.h>

#include <qstringlist.h>
#include <qvariant.h>
#include <qfileinfo.h>

#include <map>
#include <memory>

#include <cassert>

namespace {

  //
  // The usual QVariant template helpers:
  //

  // to<> is a demarshaller. It's a class b/c you can't partially
  // specialise function templates yet. However, to<> can be used as if
  // it was a function: QString s = to<QString>( myVariant );
  template <typename T> class to {};

#define MAKE_TO( type, func ) \
  template <> \
  class to< type > { \
    type m; \
  public: \
    to( const QVariant & v ) : m( v.func() ) {} \
    operator type() const { return m; } \
  }

  MAKE_TO( int, toInt );
  MAKE_TO( unsigned int, toUInt );

  template <>
  class to<KURL> {
    KURL m;
  public:
    to( const QVariant & v ) {
      m.setPath( v.toString() );
    }
    operator KURL() const { return m; }
  };

  template <typename T>
  class to< QValueList<T> > {
    QValueList<T> m;
  public:
    to( const QVariant & v ) {
      const QValueList<QVariant> vl = v.toList();
      for ( QValueList<QVariant>::const_iterator it = vl.begin(), end = vl.end() ; it != end ; ++it )
        m.push_back( to<T>( *it ) );
    }
    operator QValueList<T> () const { return m; }
  };

  template <>
  class to<KURL::List> {
    KURL::List m;
  public:
    to( const QVariant & v ) {
      // wow, KURL::List is broken... it lacks conversion from and to QVL<KURL>...
      m += to< QValueList<KURL> >( v );
    }
    operator KURL::List() const { return m; }
  };


  // from<> is the demarshaller. See to<> for why this is a class...

  template <typename T>
  struct from_helper : public QVariant {
    from_helper( const T & t ) : QVariant( t ) {}
  };

  template <typename T>
  QVariant from( const T & t ) {
    return from_helper<T>( t );
  }

  // some special types:
  template <> struct from_helper<bool> : public QVariant {
    from_helper( bool b ) : QVariant( b, int() ) {}
  };
  template <> struct from_helper<KURL> : public QVariant {
    from_helper( const KURL & url ) : QVariant( url.path() ) {}
  };
  template <typename T> struct from_helper< QValueList<T> > : public QVariant {
    from_helper( const QValueList<T> & l ) {
      QValueList<QVariant> result;
      for ( typename QValueList<T>::const_iterator it = l.begin(), end = l.end() ; it != end ; ++it )
        result.push_back( from( *it ) );
      QVariant::operator=( result );
    }
  };
  template <> struct from_helper<KURL::List> : public from_helper< QValueList<KURL> > {
    from_helper( const KURL::List & l ) : from_helper< QValueList<KURL> >( l ) {}
  };

  class ChiasmusConfigEntry : public Kleo::CryptoConfigEntry {
    unsigned int mIdx;
    QVariant mValue;
    bool mDirty;
  public:
    ChiasmusConfigEntry( unsigned int i )
      : Kleo::CryptoConfigEntry(),
        mIdx( i ), mValue( defaultValue() ), mDirty( false )
    {
      assert( i < kleo_chiasmus_config_entries_dim );
    }
    QString description() const { return i18n( kleo_chiasmus_config_entries[mIdx].description ); }
    bool isOptional() const { return kleo_chiasmus_config_entries[mIdx].is_optional; }
    bool isList() const { return kleo_chiasmus_config_entries[mIdx].is_list; }
    bool isRuntime() const { return kleo_chiasmus_config_entries[mIdx].is_runtime; }
    Level level() const { return static_cast<Level>( kleo_chiasmus_config_entries[mIdx].level ); }
    ArgType argType() const { return static_cast<ArgType>( kleo_chiasmus_config_entries[mIdx].type ); }
    bool isSet() const { return mValue != defaultValue(); }
    bool boolValue() const { return mValue.toBool(); }
    QString stringValue() const { return mValue.toString(); }
    int intValue() const { return mValue.toInt(); }
    unsigned int uintValue() const { return mValue.toUInt(); }
    KURL urlValue() const {
      if ( argType() != ArgType_Path ) return KURL( mValue.toString() );
      KURL u; u.setPath( mValue.toString() ); return u;
    }
    unsigned int numberOfTimesSet() const { return 0; }
    QStringList stringValueList() const { return mValue.toStringList(); }
    QValueList<int> intValueList() const { return to< QValueList<int> >( mValue ); }
    QValueList<unsigned int> uintValueList() const { return to< QValueList<unsigned int> >( mValue ); }
    KURL::List urlValueList() const {
      if ( argType() != ArgType_Path ) return mValue.toStringList();
      else return to<KURL::List>( mValue ); }
    void resetToDefault() { mValue = defaultValue(); mDirty = false; }
    void setBoolValue( bool value ) { setValue( QVariant( value, int() ) ); }
    void setStringValue( const QString & value ) { setValue( value ); }
    void setIntValue( int value ) { setValue( value ); }
    void setUIntValue( unsigned int value ) { setValue( value ); }
    void setURLValue( const KURL & value ) {
      if ( argType() != ArgType_Path ) setValue( value.url() );
      else setValue( value.path() );
    }
    void setNumberOfTimesSet( unsigned int ) {}
    void setStringValueList( const QStringList & value ) { setValue( value ); }
    void setIntValueList( const QValueList<int> & l ) { setValue( from( l ) ); }
    void setUIntValueList( const QValueList<unsigned int> & l ) { setValue( from( l ) ); }
    void setURLValueList( const KURL::List & l ) { setValue( from( l ) ); }
    bool isDirty() const { return mDirty; }

    QVariant value() const { return mValue; }

    void sync( KConfigBase * config ) {
      if ( !mDirty )
        return;
      mDirty = false;
      config->writeEntry( kleo_chiasmus_config_entries[mIdx].name, mValue );
    }
    void read( const KConfigBase * config ) {
      mDirty = false;
      mValue = config->readPropertyEntry( kleo_chiasmus_config_entries[mIdx].name, defaultValue() );
    }
  private:
    QVariant defaultValue() const;
    void setValue( const QVariant & value ) { mValue = value; mDirty = true; }
  };

  QVariant ChiasmusConfigEntry::defaultValue() const {
    const kleo_chiasmus_config_data & data = kleo_chiasmus_config_entries[mIdx];
    switch ( data.type ) {
    default:
      return QVariant();
    case ArgType_None:
      if ( isList() )
        return QValueList<QVariant>() << QVariant( data.defaults.boolean.value, int() );
      else
        return QVariant( data.defaults.boolean.value, int() );
    case ArgType_String:
      if ( isList() )
        return QStringList( QString::fromLatin1( data.defaults.string ) );
      else
        return QString::fromLatin1( data.defaults.string );
    case ArgType_Int:
      if ( isList() )
        return QValueList<QVariant>() << data.defaults.integer;
      else
        return data.defaults.integer;
    case ArgType_UInt:
      if ( isList() )
        return QValueList<QVariant>() << data.defaults.unsigned_integer;
      else
        return data.defaults.unsigned_integer;
    case ArgType_Path:
      if ( isList() )
        return QValueList<QVariant>() << QString::fromLatin1( data.defaults.path );
      else
        return QString::fromLatin1( data.defaults.path );
    case ArgType_URL:
    case ArgType_LDAPURL:
      if ( isList() )
        return QValueList<QVariant>() << QString::fromLatin1( data.defaults.url );
      else
        return QString::fromLatin1( data.defaults.url );
    }
  }

  class ChiasmusGeneralGroup : public Kleo::CryptoConfigGroup {
    mutable std::map<QString,ChiasmusConfigEntry*> mCache;
    mutable KConfig * mConfigObject;
  public:
    ChiasmusGeneralGroup() : Kleo::CryptoConfigGroup(), mConfigObject( 0 ) {}
    ~ChiasmusGeneralGroup() { clear(); delete mConfigObject; }
    QString description() const { return i18n( "General" ); }
    Kleo::CryptoConfigEntry::Level level() const { return Kleo::CryptoConfigEntry::Level_Basic; }
    QStringList entryList() const {
      QStringList result;
      for ( unsigned int i = 0 ; i < kleo_chiasmus_config_entries_dim ; ++i )
        result.push_back( kleo_chiasmus_config_entries[i].name );
      return result;
    }
    Kleo::CryptoConfigEntry * entry( const QString & name ) const {
      if ( ChiasmusConfigEntry * entry = mCache[name] )
        return entry;
      const KConfigGroup group( configObject(), "Chiasmus" );
      for ( unsigned int i = 0 ; i < kleo_chiasmus_config_entries_dim ; ++i )
        if ( name == kleo_chiasmus_config_entries[i].name ) {
          ChiasmusConfigEntry * entry = new ChiasmusConfigEntry( i );
          entry->read( &group );
          return mCache[name] = entry;
        }
      return 0;
    }

    void sync() {
      KConfigGroup group( configObject(), "Chiasmus" );
      for ( std::map<QString,ChiasmusConfigEntry*>::const_iterator it = mCache.begin(), end = mCache.end() ; it != end ; ++it )
        it->second->sync( &group );
      group.sync();
      clear();
    }
  private:
    KConfig * configObject() const {
      if ( !mConfigObject )
        // this is unsafe. We're a lib, used by concurrent apps.
        mConfigObject = new KConfig( "chiasmusbackendrc" );
      return mConfigObject;
    }
    void clear() {
      for ( std::map<QString,ChiasmusConfigEntry*>::const_iterator it = mCache.begin(), end = mCache.end() ; it != end ; ++it )
        delete it->second;
      mCache.clear();
    }
  };

  class ChiasmusComponent : public Kleo::CryptoConfigComponent {
    mutable ChiasmusGeneralGroup * mGeneralGroup;
  public:
    ChiasmusComponent() : Kleo::CryptoConfigComponent(), mGeneralGroup( 0 ) {}
    ~ChiasmusComponent() { delete mGeneralGroup; }

    void sync() {
      if ( mGeneralGroup )
        mGeneralGroup->sync();
    }

    QString description() const { return i18n( "Chiasmus" ); }
    QStringList groupList() const { return QStringList() << "General"; }
    Kleo::CryptoConfigGroup * group( const QString & name ) const {
      if ( name != "General" )
        return 0;
      if ( !mGeneralGroup )
        mGeneralGroup = new ChiasmusGeneralGroup();
      return mGeneralGroup;
    }
  };

}

class Kleo::ChiasmusBackend::CryptoConfig : public Kleo::CryptoConfig {
  mutable ChiasmusComponent * mComponent;
public:
  CryptoConfig() : Kleo::CryptoConfig(), mComponent( 0 ) {}
  ~CryptoConfig() { delete mComponent; }

  QStringList componentList() const { return QStringList() << "Chiasmus" ; }
  ChiasmusComponent * component( const QString & name ) const {
    if ( name != "Chiasmus" )
      return 0;
    if ( !mComponent )
      mComponent = new ChiasmusComponent();
    return mComponent;
  }
  void sync( bool ) {
    if ( mComponent )
      mComponent->sync();
  }
  void clear() { delete mComponent; mComponent = 0; }
};

class Kleo::ChiasmusBackend::Protocol : public Kleo::CryptoBackend::Protocol {
  Kleo::CryptoConfig * mCryptoConfig;
public:
  Protocol( Kleo::CryptoConfig * config )
    : Kleo::CryptoBackend::Protocol(), mCryptoConfig( config )
  {
    assert( config );
  }
  ~Protocol() {}

  QString name() const { return "Chiasmus"; }
  QString displayName() const { return i18n( "Chiasmus command line tool" ); }
  KeyListJob * keyListJob( bool, bool, bool ) const { return 0; }
  EncryptJob * encryptJob( bool, bool ) const { return 0; }
  DecryptJob * decryptJob() const { return 0; }
  SignJob * signJob( bool, bool ) const { return 0; }
  VerifyDetachedJob * verifyDetachedJob( bool ) const { return 0; }
  VerifyOpaqueJob * verifyOpaqueJob( bool ) const { return 0; }
  KeyGenerationJob * keyGenerationJob() const { return 0; }
  ImportJob * importJob() const { return 0; }
  ExportJob * publicKeyExportJob( bool ) const { return 0; }
  ExportJob * secretKeyExportJob( bool ) const { return 0; }
  DownloadJob * downloadJob( bool ) const { return 0; }
  DeleteJob * deleteJob() const { return 0; }
  SignEncryptJob * signEncryptJob( bool, bool ) const { return 0; }
  DecryptVerifyJob * decryptVerifyJob( bool ) const { return 0; }
  RefreshKeysJob * refreshKeysJob() const { return 0; }

  SpecialJob * specialJob( const char * type, const QMap<QString,QVariant> & args ) const {
    if ( qstricmp( type, "x-obtain-keys" ) == 0 && args.size() == 0 )
      return new ObtainKeysJob();
    if ( qstricmp( type, "x-encrypt" ) == 0 && args.size() == 0 )
      return new ChiasmusJob( ChiasmusJob::Encrypt );
    if ( qstricmp( type, "x-decrypt" ) == 0 && args.size() == 0 )
      return new ChiasmusJob( ChiasmusJob::Decrypt );
    kdDebug(5150) << "ChiasmusBackend::Protocol: tried to instantiate unknown job type \""
                  << type << "\"" << endl;

    return 0;
  }
};

Kleo::ChiasmusBackend * Kleo::ChiasmusBackend::self = 0;

Kleo::ChiasmusBackend::ChiasmusBackend()
  : Kleo::CryptoBackend(),
    mCryptoConfig( 0 ),
    mProtocol( 0 )
{
  self = this;
}

Kleo::ChiasmusBackend::~ChiasmusBackend() {
  self = 0;
  delete mCryptoConfig;
  delete mProtocol;
}

QString Kleo::ChiasmusBackend::name() const {
  return "Chiasmus";
}

QString Kleo::ChiasmusBackend::displayName() const {
  return i18n( "Chiasmus" );
}

Kleo::CryptoConfig * Kleo::ChiasmusBackend::config() const {
  if ( !mCryptoConfig )
    mCryptoConfig = new CryptoConfig();
  return mCryptoConfig;
}

Kleo::CryptoBackend::Protocol * Kleo::ChiasmusBackend::protocol( const char * name ) const {
  if ( qstricmp( name, "Chiasmus" ) != 0 )
    return 0;
  if ( !mProtocol )
    if ( checkForChiasmus() )
      mProtocol = new Protocol( config() );
  return mProtocol;
}

bool Kleo::ChiasmusBackend::checkForOpenPGP( QString * reason ) const {
  if ( reason )
    *reason = i18n( "Unsupported protocol \"%1\"" ).arg( "OpenPGP" );
  return false;
}

bool Kleo::ChiasmusBackend::checkForSMIME( QString * reason ) const {
  if ( reason )
    *reason = i18n( "Unsupported protocol \"%1\"" ).arg( "SMIME" );
  return false;
}

bool Kleo::ChiasmusBackend::checkForChiasmus( QString * reason ) const {

  // kills the protocol instance when we return false:
  std::auto_ptr<Protocol> tmp( mProtocol );
  mProtocol = 0;

  const CryptoConfigEntry * path = config()->entry( "Chiasmus", "General", "path" );
  assert( path ); assert( path->argType() == CryptoConfigEntry::ArgType_Path );
  const QString chiasmus = path->urlValue().path();
  const QFileInfo fi( KShell::tildeExpand( chiasmus ) );
  if ( !fi.isExecutable() ) {
    if ( reason )
      *reason = i18n( "File \"%1\" does not exist or is not executable." ).arg( chiasmus );
    return false;
  }

  const CryptoConfigEntry * useWrapper = config()->entry( "Chiasmus", "General", "use-chiasmuswrapper" );
  assert( useWrapper ); assert( useWrapper->argType() == CryptoConfigEntry::ArgType_None );
  if ( useWrapper->boolValue() == true ) {
    const CryptoConfigEntry * wrapperPath = config()->entry( "Chiasmus", "General", "chiasmuswrapper-path" );
    assert( wrapperPath ); assert( wrapperPath->argType() == CryptoConfigEntry::ArgType_Path );
    const QString wrapper = wrapperPath->urlValue().path();
    const QFileInfo wfi( KShell::tildeExpand( wrapper ) );
    if ( !wfi.isExecutable() ) {
      if ( reason )
        *reason = i18n( "File \"%1\" does not exist or is not executable." ).arg( wrapper );
      return false;
    }
  }

  // FIXME: more checks?
  mProtocol = tmp.release();
  return true;
}

bool Kleo::ChiasmusBackend::checkForProtocol( const char * name, QString * reason ) const {
  if ( qstricmp( name, "Chiasmus" ) == 0 )
    return checkForChiasmus( reason );
  if ( reason )
    *reason = i18n( "Unsupported protocol \"%1\"" ).arg( name );
  return 0;
}

bool Kleo::ChiasmusBackend::supportsProtocol( const char * name ) const {
  return qstricmp( name, "Chiasmus" ) == 0;
}

const char * Kleo::ChiasmusBackend::enumerateProtocols( int i ) const {
  return i == 0 ? "Chiasmus" : 0 ;
}
