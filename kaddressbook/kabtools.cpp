/*
    This file is part of KAddressBook.
    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kabtools.h"

#include <QtCore/QFile>

#include <kabc/addressbook.h>
#include <kabc/vcardconverter.h>
#include <kapplication.h>
#include <kdebug.h>
#include <ktempdir.h>
#include <ktoolinvocation.h>

static QString uniqueFileName( const KABC::Addressee &addressee, QStringList &existingFiles )
{
  QString name;
  QString uniquePart;

  uint number = 0;
  do {
    name = addressee.givenName() + '_' + addressee.familyName() + uniquePart + ".vcf";
    name.replace( ' ', '_' );
    name.replace( '/', '_' );

    ++number;
    uniquePart = QString( "_%1" ).arg( number );
  } while ( existingFiles.contains( name ) );

  existingFiles.append( name );

  return name;
}

void KABTools::mailVCards( const QStringList &uids, KABC::AddressBook *ab )
{
  KUrl::List urls;

  KTempDir tempDir;
  tempDir.setAutoRemove(false); //TODO: Should this be left on disk?
  if ( tempDir.status() != 0 ) {
    kWarning() << strerror( tempDir.status() );
    return;
  }

  QStringList existingFiles;
  QStringList::ConstIterator it( uids.begin() );
  const QStringList::ConstIterator endIt( uids.end() );
  for ( ; it != endIt; ++it ) {
    KABC::Addressee addressee = ab->findByUid( *it );

    if ( addressee.isEmpty() )
      continue;

    QString fileName = uniqueFileName( addressee, existingFiles );

    QString path = tempDir.name() + '/' + fileName;

    QFile file( path );

    if ( file.open( QIODevice::WriteOnly ) ) {
      KABC::VCardConverter converter;
      KABC::Addressee::List list;
      list.append( addressee );

      const QByteArray vcard = converter.createVCards( list, KABC::VCardConverter::v3_0 );

      file.write( vcard );
      file.close();

      KUrl url( path );
      url.setFileEncoding( "UTF-8" );
      urls.append( url );
    }
  }

  KToolInvocation::invokeMailer( QString(), QString(), QString(),
                      QString(),
                      QString(),
                      QString(),
                      urls.toStringList() );
}

static void mergePictures( KABC::Picture &master, const KABC::Picture &slave )
{
  if ( master.isIntern() ) {
    if ( master.data().isNull() ) {
      if ( slave.isIntern() && !slave.data().isNull() )
        master.setData( slave.data() );
      else if ( !slave.isIntern() && !slave.url().isEmpty() )
        master.setUrl( slave.url() );
    }
  } else {
    if ( master.url().isEmpty() ) {
      if ( slave.isIntern() && !slave.data().isNull() )
        master.setData( slave.data() );
      else if ( !slave.isIntern() && !slave.url().isEmpty() )
        master.setUrl( slave.url() );
    }
  }
}

KABC::Addressee KABTools::mergeContacts( const KABC::Addressee::List &list )
{
  if ( list.count() == 0 ) //emtpy
    return KABC::Addressee();
  else if ( list.count() == 1 ) // nothing to merge
    return list.first();

  KABC::Addressee masterAddressee = list.first();

  KABC::Addressee::List::ConstIterator contactIt( list.begin() );
  const KABC::Addressee::List::ConstIterator contactEndIt( list.end() );
  for ( ++contactIt; contactIt != contactEndIt; ++contactIt ) {
    // ADR + LABEL
    const KABC::Address::List addresses = (*contactIt).addresses();
    KABC::Address::List masterAddresses = masterAddressee.addresses();
    KABC::Address::List::ConstIterator addrIt( addresses.begin() );
    const KABC::Address::List::ConstIterator addrEndIt( addresses.end() );
    for ( ; addrIt != addrEndIt; ++addrIt ) {
      if ( !masterAddresses.contains( *addrIt ) )
        masterAddressee.insertAddress( *addrIt );
    }

    // BIRTHDAY
    if ( masterAddressee.birthday().isNull() && !(*contactIt).birthday().isNull() )
      masterAddressee.setBirthday( (*contactIt).birthday() );

    // CATEGORIES
    const QStringList categories = (*contactIt).categories();
    const QStringList masterCategories = masterAddressee.categories();
    QStringList newCategories( masterCategories );
    QStringList::ConstIterator it( categories.begin() );
    QStringList::ConstIterator endIt( categories.end() );
    for ( it = categories.begin(); it != endIt; ++it )
      if ( !masterCategories.contains( *it ) )
        newCategories.append( *it );
    masterAddressee.setCategories( newCategories );

    // CLASS
    if ( !masterAddressee.secrecy().isValid() && (*contactIt).secrecy().isValid() )
      masterAddressee.setSecrecy( (*contactIt).secrecy() );

    // EMAIL
    const QStringList emails = (*contactIt).emails();
    const QStringList masterEmails = masterAddressee.emails();
    endIt = emails.end();
    for ( it = emails.begin(); it != endIt; ++it )
      if ( !masterEmails.contains( *it ) )
        masterAddressee.insertEmail( *it, false );

    // FN
    if ( masterAddressee.formattedName().isEmpty() && !(*contactIt).formattedName().isEmpty() )
      masterAddressee.setFormattedName( (*contactIt).formattedName() );

    // GEO
    if ( !masterAddressee.geo().isValid() && (*contactIt).geo().isValid() )
      masterAddressee.setGeo( (*contactIt).geo() );

/*
  // KEY
*/
    // LOGO
    KABC::Picture logo = masterAddressee.logo();
    mergePictures( logo, (*contactIt).logo() );
    masterAddressee.setLogo( logo );

    // MAILER
    if ( masterAddressee.mailer().isEmpty() && !(*contactIt).mailer().isEmpty() )
      masterAddressee.setMailer( (*contactIt).mailer() );

    // N
    if ( masterAddressee.assembledName().isEmpty() && !(*contactIt).assembledName().isEmpty() )
      masterAddressee.setNameFromString( (*contactIt).assembledName() );

    // NICKNAME
    if ( masterAddressee.nickName().isEmpty() && !(*contactIt).nickName().isEmpty() )
      masterAddressee.setNickName( (*contactIt).nickName() );

    // NOTE
    if ( masterAddressee.note().isEmpty() && !(*contactIt).note().isEmpty() )
      masterAddressee.setNote( (*contactIt).note() );

    // ORG
    if ( masterAddressee.organization().isEmpty() && !(*contactIt).organization().isEmpty() )
      masterAddressee.setOrganization( (*contactIt).organization() );

    // PHOTO
    KABC::Picture photo = masterAddressee.photo();
    mergePictures( photo, (*contactIt).photo() );
    masterAddressee.setPhoto( photo );

    // PROID
    if ( masterAddressee.productId().isEmpty() && !(*contactIt).productId().isEmpty() )
      masterAddressee.setProductId( (*contactIt).productId() );

    // REV
    if ( masterAddressee.revision().isNull() && !(*contactIt).revision().isNull() )
      masterAddressee.setRevision( (*contactIt).revision() );

    // ROLE
    if ( masterAddressee.role().isEmpty() && !(*contactIt).role().isEmpty() )
      masterAddressee.setRole( (*contactIt).role() );

    // SORT-STRING
    if ( masterAddressee.sortString().isEmpty() && !(*contactIt).sortString().isEmpty() )
      masterAddressee.setSortString( (*contactIt).sortString() );

/*
  // SOUND
*/

    // TEL
    const KABC::PhoneNumber::List phones = (*contactIt).phoneNumbers();
    const KABC::PhoneNumber::List masterPhones = masterAddressee.phoneNumbers();
    KABC::PhoneNumber::List::ConstIterator phoneIt( phones.begin() );
    const KABC::PhoneNumber::List::ConstIterator phoneEndIt( phones.end() );
    for ( ; phoneIt != phoneEndIt; ++phoneIt )
      if ( !masterPhones.contains( *phoneIt ) )
        masterAddressee.insertPhoneNumber( *phoneIt );

    // TITLE
    if ( masterAddressee.title().isEmpty() && !(*contactIt).title().isEmpty() )
      masterAddressee.setTitle( (*contactIt).title() );

    // TZ
    if ( !masterAddressee.timeZone().isValid() && (*contactIt).timeZone().isValid() )
      masterAddressee.setTimeZone( (*contactIt).timeZone() );

    // UID // ignore UID

    // URL
    if ( masterAddressee.url().isEmpty() && !(*contactIt).url().isEmpty() )
      masterAddressee.setUrl( (*contactIt).url() );

    // X-
    const QStringList customs = (*contactIt).customs();
    const QStringList masterCustoms = masterAddressee.customs();
    QStringList newCustoms( masterCustoms );
    endIt = customs.end();
    for ( it = customs.begin(); it != endIt; ++it )
      if ( !masterCustoms.contains( *it ) )
        newCustoms.append( *it );
    masterAddressee.setCustoms( newCustoms );
  }

  return masterAddressee;
}
