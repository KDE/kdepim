/*  -*- mode: C++; c-file-style: "gnu"; c-basic-offset: 2 -*-
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

#define GPGCONF_FLAG_GROUP 1
#define GPGCONF_FLAG_OPTIONAL 2  // what does it mean?
#define GPGCONF_FLAG_LIST 4
#define GPGCONF_FLAG_RUNTIME 8

QGpgMECryptoConfig::QGpgMECryptoConfig( bool showErrors )
 : mComponents( 7 )
{
    mComponents.setAutoDelete( true );
    runGpgConf( showErrors );
}

QGpgMECryptoConfig::~QGpgMECryptoConfig()
{
}

void QGpgMECryptoConfig::runGpgConf( bool showErrors )
{
  // Run gpgconf --list-components to make the list of components

  KProcIO proc;
  proc << "gpgconf"; // must be in the PATH
  proc << "--list-components";

  QObject::connect( &proc, SIGNAL( readReady(KProcIO*) ),
                    this, SLOT( slotCollectStdOut(KProcIO*) ) );

  // run the process:
  int rc = 0;
  if ( !proc.start( KProcess::Block, KProcess::Stdout ) )
    rc = -1;
  else
    rc = ( proc.normalExit() ) ? proc.exitStatus() : -1 ;

  // handle errors, if any (and if requested)
  if ( showErrors && rc != 0 ) {
    QString wmsg = i18n("<qt>Failed to execute gpgconf:<br>%1</qt>").arg( strerror(rc) );
    KMessageBox::error(0, wmsg);
  }
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
  QDictIterator<Kleo::CryptoConfigComponent> it( mComponents );
  QStringList names;
  for( ; it.current(); ++it )
    names.push_back( it.currentKey() );
  return names;
}

Kleo::CryptoConfigComponent* QGpgMECryptoConfig::component( const QString& name ) const
{
  return mComponents.find( name );
}

void QGpgMECryptoConfig::clear()
{
  mComponents.clear();
}

////

QGpgMECryptoConfigComponent::QGpgMECryptoConfigComponent( QGpgMECryptoConfig*, const QString& name, const QString& description )
  : mGroups( 7 ), mDescription( description )
{
  mGroups.setAutoDelete( true );
  runGpgConf( name );
}

QGpgMECryptoConfigComponent::~QGpgMECryptoConfigComponent()
{
}

void QGpgMECryptoConfigComponent::runGpgConf( const QString& name )
{
  // Run gpgconf --list-options <component>, and create all groups and entries for that component

  KProcIO proc;
  proc << "gpgconf"; // must be in the PATH
  proc << "--list-options";
  proc << name;

  QObject::connect( &proc, SIGNAL( readReady(KProcIO*) ),
                    this, SLOT( slotCollectStdOut(KProcIO*) ) );
  mCurrentGroup = 0;

  // run the process:
  int rc = 0;
  if ( !proc.start( KProcess::Block, KProcess::Stdout ) )
    rc = -1;
  else
    rc = ( proc.normalExit() ) ? proc.exitStatus() : -1 ;

  Q_ASSERT( rc == 0 ); // Can it really be non-0, when gpg-config --list-components worked?
}

void QGpgMECryptoConfigComponent::slotCollectStdOut( KProcIO* proc )
{
  QString line;
  int result;
  while( ( result = proc->readln(line) ) != -1 ) {
    kdDebug(5150) << "GOT LINE:" << line << endl;
    // Format: NAME:FLAGS:LEVEL:DESCRIPTION:TYPE:ALT-TYPE:ARGNAME:DEFAULT:VALUE
    QStringList lst = QStringList::split( ':', line, true );
    if ( lst.count() >= 9 ) {
      int flags = lst[1].toInt();
      int level = lst[2].toInt();
      if ( flags & GPGCONF_FLAG_GROUP ) {
        mCurrentGroup = new QGpgMECryptoConfigGroup( lst[3], level );
        mGroups.insert( lst[0], mCurrentGroup );
      } else {
        // normal entry
        if ( !mCurrentGroup ) {  // first toplevel entry -> create toplevel group
          mCurrentGroup = new QGpgMECryptoConfigGroup( QString::null, 0 );
          mGroups.insert( "<nogroup>", mCurrentGroup );
        }
        // ...
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
  QDictIterator<Kleo::CryptoConfigGroup> it( mGroups );
  QStringList names;
  for( ; it.current(); ++it )
    names.push_back( it.currentKey() );
  return names;
}

Kleo::CryptoConfigGroup* QGpgMECryptoConfigComponent::group(const QString& name ) const
{
  return mGroups.find( name );
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
  QDictIterator<Kleo::CryptoConfigEntry> it( mEntries );
  QStringList names;
  for( ; it.current(); ++it )
    names.push_back( it.currentKey() );
  return names;
}

Kleo::CryptoConfigEntry* QGpgMECryptoConfigGroup::entry( const QString& name ) const
{
  return mEntries.find( name );
}

#include "qgpgmecryptoconfig.moc"
