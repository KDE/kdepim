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

#include "kleo_export.h"
#include "kleo/cryptoconfig.h"

#include <QHash>
#include <QStringList>
#include <QObject>
#include <QVariant>
class K3ProcIO;

class QGpgMECryptoConfigComponent;
class QGpgMECryptoConfigEntry;
/**
 * CryptoConfig implementation around the gpgconf command-line tool
 * For method docu, see kleo/cryptoconfig.h
 */
class KLEO_EXPORT QGpgMECryptoConfig : public QObject, public Kleo::CryptoConfig {

  Q_OBJECT
public:
  /**
   * Constructor
   */
  QGpgMECryptoConfig();
  virtual ~QGpgMECryptoConfig();

  virtual QStringList componentList() const;

  virtual Kleo::CryptoConfigComponent* component( const QString& name ) const;

  virtual void clear();
  virtual void sync( bool runtime );

private slots:
  void slotCollectStdOut( K3ProcIO* proc );
private:
  /// @param showErrors if true, a messagebox will be shown if e.g. gpgconf wasn't found
  void runGpgConf( bool showErrors );

private:
  QHash<QString, QGpgMECryptoConfigComponent*> mComponents;
  bool mParsed;
};

class QGpgMECryptoConfigGroup;

/// For docu, see kleo/cryptoconfig.h
class QGpgMECryptoConfigComponent : public QObject, public Kleo::CryptoConfigComponent {

  Q_OBJECT
public:
  QGpgMECryptoConfigComponent( QGpgMECryptoConfig*, const QString& name, const QString& description );
  ~QGpgMECryptoConfigComponent();

  QString name() const { return mName; }
  QString iconName() const { return mName; }
  QString description() const { return mDescription; }
  QStringList groupList() const;
  Kleo::CryptoConfigGroup* group( const QString& name ) const;

  void sync( bool runtime );

private slots:
  void slotCollectStdOut( K3ProcIO* proc );
private:
  void runGpgConf();

private:
  QHash<QString,QGpgMECryptoConfigGroup*> mGroups;
  QString mName;
  QString mDescription;
  QGpgMECryptoConfigGroup* mCurrentGroup; // during parsing
  QString mCurrentGroupName; // during parsing
};

class QGpgMECryptoConfigGroup : public Kleo::CryptoConfigGroup {

public:
  QGpgMECryptoConfigGroup( const QString & name, const QString& description, int level );
  ~QGpgMECryptoConfigGroup();

  QString name() const { return mName; }
  QString iconName() const { return QString(); }
  QString description() const { return mDescription; }
  Kleo::CryptoConfigEntry::Level level() const { return mLevel; }
  QStringList entryList() const;
  Kleo::CryptoConfigEntry* entry( const QString& name ) const;

private:
  friend class QGpgMECryptoConfigComponent; // it adds the entries
  QHash<QString,QGpgMECryptoConfigEntry*> mEntries;
  QString mName;
  QString mDescription;
  Kleo::CryptoConfigEntry::Level mLevel;
};

class QGpgMECryptoConfigEntry : public Kleo::CryptoConfigEntry {
public:
  QGpgMECryptoConfigEntry( const QStringList& parsedLine );
  ~QGpgMECryptoConfigEntry();

  QString name() const { return mName; }
  QString description() const { return mDescription; }
  bool isOptional() const;
  bool isReadOnly() const;
  bool isList() const;
  bool isRuntime() const;
  Level level() const { return static_cast<Level>( mLevel ); }
  ArgType argType() const { return static_cast<ArgType>( mArgType ); }
  bool isSet() const;
  bool boolValue() const;
  QString stringValue() const;
  int intValue() const;
  unsigned int uintValue() const;
  KUrl urlValue() const;
  unsigned int numberOfTimesSet() const;
  QStringList stringValueList() const;
  QList<int> intValueList() const;
  QList<unsigned int> uintValueList() const;
  KUrl::List urlValueList() const;
  void resetToDefault();
  void setBoolValue( bool );
  void setStringValue( const QString& );
  void setIntValue( int );
  void setUIntValue( unsigned int );
  void setURLValue( const KUrl& );
  void setNumberOfTimesSet( unsigned int );
  void setStringValueList( const QStringList& );
  void setIntValueList( const QList<int>& );
  void setUIntValueList( const QList<unsigned int>& );
  void setURLValueList( const KUrl::List& );
  bool isDirty() const { return mDirty; }

  void setDirty( bool b );
  QString outputString() const;

protected:
  bool isStringType() const;
  QVariant stringToValue( const QString& value, bool unescape ) const;
  QString toString( bool escape ) const;
private:
  QString mName;
  QString mDescription;
  QVariant mDefaultValue;
  QVariant mValue;
  uint mFlags : 8; // bitfield with 8 bits
  uint mLevel : 3; // max is 4 (2, in fact) -> 3 bits
  uint mRealArgType : 6; // max is 33 -> 6 bits
  uint mArgType : 3; // max is 6 (ArgType enum) -> 3 bits;
  uint mDirty : 1;
  uint mSet : 1;
};

#endif /* KLEO_QGPGMECRYPTOCONFIG_H */
