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

#include <assert.h>

// TODO: runtime changes from other apps? Is that possible to support (other than manual clear()?)

#define GPGCONF_FLAG_GROUP 1
#define GPGCONF_FLAG_OPTIONAL 2  // what does it mean? (asked in aegypten issue89)
#define GPGCONF_FLAG_LIST 4
#define GPGCONF_FLAG_RUNTIME 8

QGpgMECryptoConfig::QGpgMECryptoConfig( bool showErrors )
 : mComponents( 7 ), mParsed( false )
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
    const_cast<QGpgMECryptoConfig*>( this )->runGpgConf( false );
  QDictIterator<Kleo::CryptoConfigComponent> it( mComponents );
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

void QGpgMECryptoConfig::clear()
{
  mComponents.clear();
  mParsed = false; // next call to componentList/component will need to run gpgconf again
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

  if( rc != 0 ) // Can it really be non-0, when gpg-config --list-components worked?
    kdWarning(5150) << k_funcinfo << ":" << strerror( rc ) << endl;
}

void QGpgMECryptoConfigComponent::slotCollectStdOut( KProcIO* proc )
{
  QString line;
  int result;
  while( ( result = proc->readln(line) ) != -1 ) {
    //kdDebug(5150) << "GOT LINE:" << line << endl;
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

////

static QString gpgconf_unescape( const QString& str )
{
  // Looks like it's the same rules as KURL.
  return KURL::decode_string( str );
}

QGpgMECryptoConfigEntry::QGpgMECryptoConfigEntry( const QStringList& parsedLine )
{
  // Format: NAME:FLAGS:LEVEL:DESCRIPTION:TYPE:ALT-TYPE:ARGNAME:DEFAULT:VALUE
  assert( parsedLine.count() >= 9 ); // called checked for it already
  QStringList::const_iterator it = parsedLine.begin();
  ++it; // skip name, stored in group
  mFlags = (*it++).toInt();
  mLevel = (*it++).toInt();
  mDescription = (*it++);
  int dataType = (*it++).toInt();
  if ( dataType <= DataType_URL ) // we support all types up to url (5)
    mDataType = dataType;
  else if ( !(*it).isEmpty() ) {
    dataType = (*it).toInt(); // use ALT-TYPE
    if ( dataType <= DataType_URL )
      mDataType = dataType;
    else
      kdWarning(5150) << "Unsupported datatype: " << parsedLine[4] << " : " << *it << endl;
  }
  ++it; // skip alt-type
  ++it; // skip argname (what is it good for?) (asked in aegypten issue89)
  QString value = *it++; // get default value
  if ( !(*it).isEmpty() )
    value = *it; // a real value was set

  bool isString = ( dataType == Kleo::CryptoConfigEntry::DataType_String
                    || dataType == Kleo::CryptoConfigEntry::DataType_Path
                    || dataType == Kleo::CryptoConfigEntry::DataType_URL );
  if ( isString ) {
    if ( value.isEmpty() )
      mValue = QVariant( QString::null ); // not set  [ok with lists too?]
    else {
      Q_ASSERT( value[0] == '"' ); // see README.gpgconf
      value = value.mid( 1 );
    }
  }

  if ( !mValue.isValid() ) {
    if ( isList() ) {
      QValueList<QVariant> lst;
      QStringList items = QStringList::split( ',', value );
      for( QStringList::Iterator valit = items.begin(); valit != items.end(); ++valit ) {
        lst << QVariant( gpgconf_unescape( *valit ) );
      }
      mValue = lst;
    }
    else
      mValue = QVariant( gpgconf_unescape( value ) );
  }
}

QGpgMECryptoConfigEntry::~QGpgMECryptoConfigEntry()
{
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

bool QGpgMECryptoConfigEntry::boolValue() const
{
  Q_ASSERT( mDataType == DataType_Bool );
  Q_ASSERT( !isList() );
  return mValue.toBool();
}

QString QGpgMECryptoConfigEntry::stringValue() const
{
  Q_ASSERT( mDataType == DataType_String || mDataType == DataType_Path || mDataType == DataType_URL );
  Q_ASSERT( !isList() );
  return mValue.toString();
}

int QGpgMECryptoConfigEntry::intValue() const
{
  Q_ASSERT( mDataType == DataType_Int );
  Q_ASSERT( !isList() );
  return mValue.toInt();
}

unsigned int QGpgMECryptoConfigEntry::uintValue() const
{
  Q_ASSERT( mDataType == DataType_UInt );
  Q_ASSERT( !isList() );
  return mValue.toUInt();
}

KURL QGpgMECryptoConfigEntry::urlValue() const
{
  Q_ASSERT( mDataType == DataType_Path || mDataType == DataType_URL );
  Q_ASSERT( !isList() );
  QString str = mValue.toString();
  if ( mDataType == DataType_URL )
    return KURL( str );
  KURL url;
  url.setPath( str );
  return url;
}

QValueList<bool> QGpgMECryptoConfigEntry::boolValueList() const
{
  Q_ASSERT( mDataType == DataType_Bool );
  Q_ASSERT( isList() );
  QValueList<bool> ret;
  QValueList<QVariant> lst = mValue.toList();
  for( QValueList<QVariant>::Iterator it = lst.begin(); it != lst.end(); ++it ) {
    ret.append( (*it).toBool() );
  }
  return ret;
}

QStringList QGpgMECryptoConfigEntry::stringValueList() const
{
  Q_ASSERT( mDataType == DataType_String || mDataType == DataType_Path || mDataType == DataType_URL );
  Q_ASSERT( isList() );
  return mValue.toStringList();
}

QValueList<int> QGpgMECryptoConfigEntry::intValueList() const
{
  Q_ASSERT( mDataType == DataType_Int );
  Q_ASSERT( isList() );
  QValueList<int> ret;
  QValueList<QVariant> lst = mValue.toList();
  for( QValueList<QVariant>::Iterator it = lst.begin(); it != lst.end(); ++it ) {
    ret.append( (*it).toInt() );
  }
  return ret;
}

QValueList<unsigned int> QGpgMECryptoConfigEntry::uintValueList() const
{
  Q_ASSERT( mDataType == DataType_UInt );
  Q_ASSERT( isList() );
  QValueList<unsigned int> ret;
  QValueList<QVariant> lst = mValue.toList();
  for( QValueList<QVariant>::Iterator it = lst.begin(); it != lst.end(); ++it ) {
    ret.append( (*it).toUInt() );
  }
  return ret;
}

KURL::List QGpgMECryptoConfigEntry::urlValueList() const
{
  Q_ASSERT( mDataType == DataType_Path || mDataType == DataType_URL );
  Q_ASSERT( isList() );
  QStringList lst = mValue.toStringList();
  if ( mDataType == DataType_URL )
    return KURL::List( lst );

  KURL::List ret;
  for( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
    KURL url;
    url.setPath( *it );
    ret << url;
  }
  return ret;
}

void QGpgMECryptoConfigEntry::setBoolValue( bool, bool /*runtime*/ )
{

}

void QGpgMECryptoConfigEntry::setStringValue( const QString&, bool /*runtime*/ )
{

}

void QGpgMECryptoConfigEntry::setIntValue( int, bool /*runtime*/ )
{

}

void QGpgMECryptoConfigEntry::setUIntValue( unsigned int, bool /*runtime*/ )
{

}

void QGpgMECryptoConfigEntry::setURLValue( const KURL&, bool /*runtime*/ )
{

}

void QGpgMECryptoConfigEntry::setBoolValueList( QValueList<bool>, bool /*runtime*/ )
{

}

void QGpgMECryptoConfigEntry::setStringValueList( const QStringList&, bool /*runtime*/ )
{

}

void QGpgMECryptoConfigEntry::setIntValueList( const QValueList<int>&, bool /*runtime*/ )
{

}

void QGpgMECryptoConfigEntry::setUIntValueList( const QValueList<unsigned int>&, bool /*runtime*/ )
{

}

void QGpgMECryptoConfigEntry::setURLValueList( const KURL::List&, bool /*runtime*/ )
{

}

#include "qgpgmecryptoconfig.moc"
