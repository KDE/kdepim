/*  -*- mode: C++; c-file-style: "gnu"; c-basic-offset: 2 -*-
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

#ifndef KLEO_QGPGMECRYPTOCONFIG_H
#define KLEO_QGPGMECRYPTOCONFIG_H

#include <kleo/cryptoconfig.h>
#include <qdict.h>
#include <qstringlist.h>
#include <qobject.h>
class KProcIO;

/**
 * CryptoConfig implementation around the gpgconf command-line tool
 * For method docu, see kleo/cryptoconfig.h
 */
class QGpgMECryptoConfig : public QObject, public Kleo::CryptoConfig {

  Q_OBJECT
public:
  /**
   * Constructor
   * @param showErrors if true, a messagebox will be shown if e.g. gpgconf wasn't found
   */
  QGpgMECryptoConfig( bool showErrors = true );
  virtual ~QGpgMECryptoConfig();

  virtual QStringList componentList() const;

  virtual Kleo::CryptoConfigComponent* component( const QString& name ) const;

  virtual void clear();

private slots:
  void slotCollectStdOut( KProcIO* proc );
private:
  void runGpgConf( bool showErrors );

private:
  QDict<Kleo::CryptoConfigComponent> mComponents;
};

class QGpgMECryptoConfigGroup;

/// For docu, see kleo/cryptoconfig.h
class QGpgMECryptoConfigComponent : public QObject, public Kleo::CryptoConfigComponent {

  Q_OBJECT
public:
  QGpgMECryptoConfigComponent( QGpgMECryptoConfig*, const QString& name, const QString& description );
  virtual ~QGpgMECryptoConfigComponent();

  virtual QString description() const { return mDescription; }
  virtual QStringList groupList() const;
  virtual Kleo::CryptoConfigGroup* group( const QString& name ) const;

private slots:
  void slotCollectStdOut( KProcIO* proc );
private:
  void runGpgConf( const QString& name );

private:
  QDict<Kleo::CryptoConfigGroup> mGroups;
  QString mDescription;
  QGpgMECryptoConfigGroup* mCurrentGroup; // during parsing
};

class QGpgMECryptoConfigGroup : public Kleo::CryptoConfigGroup {

public:
  QGpgMECryptoConfigGroup( const QString& description, int level );
  virtual ~QGpgMECryptoConfigGroup() {}

  virtual QString description() const { return mDescription; }
  virtual Kleo::CryptoConfigEntry::Level level() const { return mLevel; }
  virtual QStringList entryList() const;
  virtual Kleo::CryptoConfigEntry* entry( const QString& name ) const;

private:
  friend class QGpgMECryptoConfigComponent; // it adds the entries
  QDict<Kleo::CryptoConfigEntry> mEntries;
  QString mDescription;
  Kleo::CryptoConfigEntry::Level mLevel;
};

#endif /* KLEO_QGPGMECRYPTOCONFIG_H */
