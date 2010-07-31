/*
    qgpgmecryptoconfig.h

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

#ifndef KLEO_QGPGMECRYPTOCONFIG_H
#define KLEO_QGPGMECRYPTOCONFIG_H

#include <kleo/cryptoconfig.h>
#include <tqdict.h>
#include <tqstringlist.h>
#include <tqobject.h>
#include <tqvariant.h>
class KProcIO;

class QGpgMECryptoConfigComponent;
class QGpgMECryptoConfigEntry;
/**
 * CryptoConfig implementation around the gpgconf command-line tool
 * For method docu, see kleo/cryptoconfig.h
 */
class QGpgMECryptoConfig : public TQObject, public Kleo::CryptoConfig {

  Q_OBJECT
public:
  /**
   * Constructor
   */
  QGpgMECryptoConfig();
  virtual ~QGpgMECryptoConfig();

  virtual TQStringList componentList() const;

  virtual Kleo::CryptoConfigComponent* component( const TQString& name ) const;

  virtual void clear();
  virtual void sync( bool runtime );

private slots:
  void slotCollectStdOut( KProcIO* proc );
private:
  /// @param showErrors if true, a messagebox will be shown if e.g. gpgconf wasn't found
  void runGpgConf( bool showErrors );

private:
  TQDict<QGpgMECryptoConfigComponent> mComponents;
  bool mParsed;
};

class QGpgMECryptoConfigGroup;

/// For docu, see kleo/cryptoconfig.h
class QGpgMECryptoConfigComponent : public TQObject, public Kleo::CryptoConfigComponent {

  Q_OBJECT
public:
  QGpgMECryptoConfigComponent( QGpgMECryptoConfig*, const TQString& name, const TQString& description );
  ~QGpgMECryptoConfigComponent();

  TQString name() const { return mName; }
  TQString iconName() const { return mName; }
  TQString description() const { return mDescription; }
  TQStringList groupList() const;
  Kleo::CryptoConfigGroup* group( const TQString& name ) const;

  void sync( bool runtime );

private slots:
  void slotCollectStdOut( KProcIO* proc );
private:
  void runGpgConf();

private:
  TQDict<QGpgMECryptoConfigGroup> mGroups;
  TQString mName;
  TQString mDescription;
  QGpgMECryptoConfigGroup* mCurrentGroup; // during parsing
  TQString mCurrentGroupName; // during parsing
};

class QGpgMECryptoConfigGroup : public Kleo::CryptoConfigGroup {

public:
  QGpgMECryptoConfigGroup( const TQString & name, const TQString& description, int level );
  ~QGpgMECryptoConfigGroup() {}

  TQString name() const { return mName; }
  TQString iconName() const { return TQString::null; }
  TQString description() const { return mDescription; }
  Kleo::CryptoConfigEntry::Level level() const { return mLevel; }
  TQStringList entryList() const;
  Kleo::CryptoConfigEntry* entry( const TQString& name ) const;

private:
  friend class QGpgMECryptoConfigComponent; // it adds the entries
  TQDict<QGpgMECryptoConfigEntry> mEntries;
  TQString mName;
  TQString mDescription;
  Kleo::CryptoConfigEntry::Level mLevel;
};

class QGpgMECryptoConfigEntry : public Kleo::CryptoConfigEntry {
public:
  QGpgMECryptoConfigEntry( const TQStringList& parsedLine );
  ~QGpgMECryptoConfigEntry();

  TQString name() const { return mName; }
  TQString description() const { return mDescription; }
  bool isOptional() const;
  bool isReadOnly() const;
  bool isList() const;
  bool isRuntime() const;
  Level level() const { return static_cast<Level>( mLevel ); }
  ArgType argType() const { return static_cast<ArgType>( mArgType ); }
  bool isSet() const;
  bool boolValue() const;
  TQString stringValue() const;
  int intValue() const;
  unsigned int uintValue() const;
  KURL urlValue() const;
  unsigned int numberOfTimesSet() const;
  TQStringList stringValueList() const;
  TQValueList<int> intValueList() const;
  TQValueList<unsigned int> uintValueList() const;
  KURL::List urlValueList() const;
  void resetToDefault();
  void setBoolValue( bool );
  void setStringValue( const TQString& );
  void setIntValue( int );
  void setUIntValue( unsigned int );
  void setURLValue( const KURL& );
  void setNumberOfTimesSet( unsigned int );
  void setStringValueList( const TQStringList& );
  void setIntValueList( const TQValueList<int>& );
  void setUIntValueList( const TQValueList<unsigned int>& );
  void setURLValueList( const KURL::List& );
  bool isDirty() const { return mDirty; }

  void setDirty( bool b );
  TQString outputString() const;

protected:
  bool isStringType() const;
  TQVariant stringToValue( const TQString& value, bool unescape ) const;
  TQString toString( bool escape ) const;
private:
  TQString mName;
  TQString mDescription;
  TQVariant mDefaultValue;
  TQVariant mValue;
  uint mFlags : 8; // bitfield with 8 bits
  uint mLevel : 3; // max is 4 (2, in fact) -> 3 bits
  uint mRealArgType : 6; // max is 33 -> 6 bits
  uint mArgType : 3; // max is 6 (ArgType enum) -> 3 bits;
  uint mDirty : 1;
  uint mSet : 1;
};

#endif /* KLEO_QGPGMECRYPTOCONFIG_H */
