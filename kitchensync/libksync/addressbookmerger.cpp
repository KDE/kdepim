/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003,2004 Holger Freyther <zecke@handhelds.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "addressbookmerger.h"

#include "addressbooksyncee.h"

#include <kabc/addressee.h>
#include <kstaticdeleter.h>

namespace KSync {
namespace AddressBookMergerInternal {


typedef void (*merge)(KABC::Addressee&, const KABC::Addressee& );
typedef QMap<int, merge> MergeMap;

static MergeMap* _mergeMap= 0;
static KStaticDeleter<MergeMap> mergeMapDeleter;

/* merge functions */
static void mergeFamily    ( KABC::Addressee&, const KABC::Addressee& );
static void mergeGiven     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeAdditionalName( KABC::Addressee&, const KABC::Addressee& );
static void mergePrefix    ( KABC::Addressee&, const KABC::Addressee& );
static void mergeSuffix    ( KABC::Addressee&, const KABC::Addressee& );
static void mergeNickName      ( KABC::Addressee&, const KABC::Addressee& );
static void mergeBirthDay      ( KABC::Addressee&, const KABC::Addressee& );
static void mergeHomeAddress   ( KABC::Addressee&, const KABC::Addressee& );
static void mergeBusinessAddress ( KABC::Addressee&, const KABC::Addressee& );
static void mergeTimeZone      ( KABC::Addressee&, const KABC::Addressee& );
static void mergeGeo       ( KABC::Addressee&, const KABC::Addressee& );
static void mergeTitle     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeRole      ( KABC::Addressee&, const KABC::Addressee& );
static void mergeOrganization  ( KABC::Addressee&, const KABC::Addressee& );
static void mergeNote      ( KABC::Addressee&, const KABC::Addressee& );
static void mergeUrl       ( KABC::Addressee&, const KABC::Addressee& );
static void mergeSecrecy   ( KABC::Addressee&, const KABC::Addressee& );
static void mergePicture   ( KABC::Addressee&, const KABC::Addressee& );
static void mergeSound     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeAgent     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeHomeTelephoneNumber( KABC::Addressee&,
                                      const KABC::Addressee& );
static void mergeOfficeTelephoneNumber( KABC::Addressee&,
                                        const KABC::Addressee& );
static void mergeMessenger ( KABC::Addressee&, const KABC::Addressee& );
static void mergePreferredNumber( KABC::Addressee&, const KABC::Addressee& );
static void mergeVoice     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeFax       ( KABC::Addressee&, const KABC::Addressee& );
static void mergeCellPhone ( KABC::Addressee&, const KABC::Addressee& );
static void mergeVideo     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeMailbox   ( KABC::Addressee&, const KABC::Addressee& );
static void mergeModem     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeCarPhone  ( KABC::Addressee&, const KABC::Addressee& );
static void mergeISDN      ( KABC::Addressee&, const KABC::Addressee& );
static void mergePCS       ( KABC::Addressee&, const KABC::Addressee& );
static void mergePager     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeHomeFax   ( KABC::Addressee&, const KABC::Addressee& );
static void mergeWorkFax   ( KABC::Addressee&, const KABC::Addressee& );
static void mergeOtherTelephone( KABC::Addressee&, const KABC::Addressee& );
static void mergeCategory  ( KABC::Addressee&, const KABC::Addressee& );
static void mergeKeys      ( KABC::Addressee&, const KABC::Addressee& );
static void mergeCustom    ( KABC::Addressee&, const KABC::Addressee& );
static void mergeLogo      ( KABC::Addressee&, const KABC::Addressee& );
static void mergeEmails    ( KABC::Addressee&, const KABC::Addressee& );
static QStringList mergeList( const QStringList& entry, const QStringList& other );

MergeMap* mergeMap()
{
  if (!_mergeMap ) {
    /* now fill it with functions.... */
    mergeMapDeleter.setObject( _mergeMap,  new MergeMap );
    _mergeMap->insert(AddressBookMerger::FamilyName, mergeFamily );
    _mergeMap->insert(AddressBookMerger::GivenName,  mergeGiven );
    _mergeMap->insert(AddressBookMerger::AdditionalName, mergeAdditionalName );
    _mergeMap->insert(AddressBookMerger::Prefix, mergePrefix );
    _mergeMap->insert(AddressBookMerger::Suffix, mergeSuffix );
    _mergeMap->insert(AddressBookMerger::NickName, mergeNickName );
    _mergeMap->insert(AddressBookMerger::Birthday, mergeBirthDay );
    _mergeMap->insert(AddressBookMerger::HomeAddress, mergeHomeAddress );
    _mergeMap->insert(AddressBookMerger::BusinessAddress, mergeBusinessAddress );
    _mergeMap->insert(AddressBookMerger::TimeZone, mergeTimeZone );
    _mergeMap->insert(AddressBookMerger::Geo, mergeGeo );
    _mergeMap->insert(AddressBookMerger::Title, mergeTitle );
    _mergeMap->insert(AddressBookMerger::Role, mergeRole );
    _mergeMap->insert(AddressBookMerger::Organization, mergeOrganization );
    _mergeMap->insert(AddressBookMerger::Note, mergeNote );
    _mergeMap->insert(AddressBookMerger::Url, mergeUrl );
    _mergeMap->insert(AddressBookMerger::Secrecy, mergeSecrecy );
    _mergeMap->insert(AddressBookMerger::Picture, mergePicture );
    _mergeMap->insert(AddressBookMerger::Sound, mergeSound );
    _mergeMap->insert(AddressBookMerger::Agent, mergeAgent );
    _mergeMap->insert(AddressBookMerger::HomeNumbers, mergeHomeTelephoneNumber );
    _mergeMap->insert(AddressBookMerger::OfficeNumbers, mergeOfficeTelephoneNumber );
    _mergeMap->insert(AddressBookMerger::Messenger, mergeMessenger );
    _mergeMap->insert(AddressBookMerger::PreferredNumber, mergePreferredNumber );
    _mergeMap->insert(AddressBookMerger::Voice, mergeVoice );
    _mergeMap->insert(AddressBookMerger::Fax, mergeFax );
    _mergeMap->insert(AddressBookMerger::Cell, mergeCellPhone );
    _mergeMap->insert(AddressBookMerger::Video, mergeVideo );
    _mergeMap->insert(AddressBookMerger::Mailbox, mergeMailbox );
    _mergeMap->insert(AddressBookMerger::Modem, mergeModem );
    _mergeMap->insert(AddressBookMerger::CarPhone, mergeCarPhone );
    _mergeMap->insert(AddressBookMerger::ISDN, mergeISDN );
    _mergeMap->insert(AddressBookMerger::PCS, mergePCS );
    _mergeMap->insert(AddressBookMerger::Pager, mergePager );
    _mergeMap->insert(AddressBookMerger::HomeFax, mergeHomeFax );
    _mergeMap->insert(AddressBookMerger::WorkFax, mergeWorkFax );
    _mergeMap->insert(AddressBookMerger::OtherTel, mergeOtherTelephone );
    _mergeMap->insert(AddressBookMerger::Category, mergeCategory );
    _mergeMap->insert(AddressBookMerger::Custom, mergeCustom );
    _mergeMap->insert(AddressBookMerger::Keys, mergeKeys );
    _mergeMap->insert(AddressBookMerger::Logo, mergeLogo );
    _mergeMap->insert(AddressBookMerger::Email, mergeEmails );
  }
  return _mergeMap;
}
    /* merge functions */
static void mergeFamily    ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setFamilyName( other.familyName() );
}

static void mergeGiven     ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setGivenName( other.givenName() );
}

static void mergeAdditionalName( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setAdditionalName( other.additionalName() );
}

static void mergePrefix    ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setPrefix( other.prefix() );
}

static void mergeSuffix    ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setSuffix( other.suffix() );
}

static void mergeNickName( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setNickName( other.nickName() );
}

static void mergeBirthDay( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setBirthday( other.birthday() );
}

static void mergeHomeAddress( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertAddress( other.address( KABC::Address::Home ) );
}

static void mergeBusinessAddress( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertAddress( other.address( KABC::Address::Work ) );
}

static void mergeTimeZone( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setTimeZone( other.timeZone() );
}

static void mergeGeo       ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setGeo( other.geo() );
}

static void mergeTitle     ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setTitle( other.title() );
}

static void mergeRole      ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setRole( other.role() );
}

static void mergeOrganization( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setOrganization( other.organization() );
}

static void mergeNote      ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setNote( other.note() );
}

static void mergeUrl       ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setUrl( other.url() );
}

static void mergeSecrecy   ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setSecrecy( other.secrecy() );
}

static void mergePicture   ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setPhoto( other.photo() );
}

static void mergeSound     ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setSound( other.sound() );
}

static void mergeAgent     ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setAgent( other.agent() );
}

static void mergeHomeTelephoneNumber( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Home ) );
}

static void mergeOfficeTelephoneNumber( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Work ) );
}

static void mergeMessenger ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Msg ) );
}

static void mergePreferredNumber( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Pref ) );
}

static void mergeVoice     ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Voice ) );
}

static void mergeFax       ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Fax ) );
}

static void mergeCellPhone ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Cell ) );
}

static void mergeVideo     ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Video ) );
}

static void mergeMailbox   ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Bbs ) );
}

static void mergeModem     ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Modem ) );
}

static void mergeCarPhone  ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Car ) );
}

static void mergeISDN      ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Isdn ) );
}

static void mergePCS       ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Pcs ) );
}

static void mergePager     ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Pager ) );
}

static void mergeHomeFax   ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax ) );
}

static void mergeWorkFax   ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax ) );
}

static void mergeOtherTelephone ( KABC::Addressee&, const KABC::Addressee& )
{}

static void mergeCategory( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setCategories( other.categories() );
}

static void mergeKeys      ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setKeys( other.keys() );
}

static void mergeCustom    ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setCustoms( AddressBookMergerInternal::mergeList( entry.customs(), other.customs() ) );
}

static void mergeLogo      ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  entry.setLogo( other.logo() );
}

static void mergeEmails    ( KABC::Addressee& entry, const KABC::Addressee& other)
{
  QString pref = entry.preferredEmail();
  entry.setEmails( other.emails() );
  entry.insertEmail( pref, true );
}

}




AddressBookMerger::AddressBookMerger( const QBitArray &supp)
  : mSupports( supp )
{
  setSynceeType( QString::fromLatin1( "AddressBookSyncee" ) );
}

AddressBookMerger::~AddressBookMerger()
{}


bool AddressBookMerger::merge( SyncEntry* _entry, SyncEntry* _other )
{
  /*
   * Check if we're from the same type
   */
  if ( !sameType( _entry, _other, QString::fromLatin1("AddressBookSyncEntry") ) )
    return false;

  /*
   * Now we can safely cast
   */
  AddressBookSyncEntry *entry, *other;
  entry = static_cast<AddressBookSyncEntry*>( _entry );
  other = static_cast<AddressBookSyncEntry*>( _other );

  /*
   * Create the Map, and then start
   */
  AddressBookMergerInternal::MergeMap::Iterator it;
  AddressBookMergerInternal::MergeMap* ma = AddressBookMergerInternal::mergeMap();


  /*
   * See what the other support
   * If it doesn't have a Merger we will assume it supports
   * everything
   */
  QBitArray otherSupport;
  if ( other->syncee() && other->syncee()->merger() )
    otherSupport = (otherMerger<AddressBookMerger>( other ))->mSupports;
  else {
    otherSupport = QBitArray( mSupports.size() );
    otherSupport.fill( true );
  }

  for ( uint i = 0; i < mSupports.size() && i < otherSupport.size(); ++i )
    /* we don't know it, but the other do so merge */
    if ( otherSupport[i] && !mSupports[i] ) {
      it = ma->find( i );
      if ( it != ma->end() )
        (*it.data())(entry->mAddressee, other->mAddressee );
    }

  /*  now merge custom entries */
  AddressBookMergerInternal::mergeCustom( entry->mAddressee, other->mAddressee );

  return true;
}


















namespace AddressBookMergerInternal {

static QStringList mergeList( const QStringList& entry, const QStringList& other ) {
    QStringList list = entry;

    QStringList::ConstIterator it;
    for (it = other.begin(); it != other.end(); ++it ) {
        if (!list.contains( (*it) ) )
            list << (*it);
    }

    return list;
}

}
}
