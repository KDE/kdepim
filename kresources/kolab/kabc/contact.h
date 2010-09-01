/*
    This file is part of libkabc and/or kaddressbook.
    Copyright (c) 2002 - 2004 Klarälvdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

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

#ifndef KOLABCONTACT_H
#define KOLABCONTACT_H

#include <kolabbase.h>
#include <tqimage.h>

namespace KABC {
  class Addressee;
  class ResourceKolab;
  class Picture;
  class Sound;
}

namespace Kolab {

class Contact : public KolabBase {
public:
  struct PhoneNumber {
  public:
    TQString type;
    TQString number;
  };

  struct Address {
  public:
    Address() : kdeAddressType( -1 )
    {
    }
    int kdeAddressType; // KABC::Address::Type
    TQString type;       // kolab-compliant address type: home, work or other
    TQString street;
    TQString pobox;
    TQString locality;
    TQString region;
    TQString postalCode;
    TQString country;
  };

  explicit Contact( const KABC::Addressee* address );
  Contact( const TQString& xml, KABC::ResourceKolab* resource, const TQString& subResource, Q_UINT32 sernum );
  ~Contact();

  void saveTo( KABC::Addressee* address );

  TQString type() const { return "Contact"; }

  void setGivenName( const TQString& name );
  TQString givenName() const;

  void setMiddleNames( const TQString& names );
  TQString middleNames() const;

  void setLastName( const TQString& name );
  TQString lastName() const;

  void setFullName( const TQString& name );
  TQString fullName() const;

  void setInitials( const TQString& initials );
  TQString initials() const;

  void setPrefix( const TQString& prefix );
  TQString prefix() const;

  void setSuffix( const TQString& suffix );
  TQString suffix() const;

  void setRole( const TQString& role );
  TQString role() const;

  void setFreeBusyUrl( const TQString& fbUrl );
  TQString freeBusyUrl() const;

  void setOrganization( const TQString& organization );
  TQString organization() const;

  void setWebPage( const TQString& url );
  TQString webPage() const;

  void setIMAddress( const TQString& imAddress );
  TQString imAddress() const;

  void setDepartment( const TQString& department );
  TQString department() const;

  void setOfficeLocation( const TQString& location );
  TQString officeLocation() const;

  void setProfession( const TQString& profession );
  TQString profession() const;

  void setJobTitle( const TQString& title );
  TQString jobTitle() const;

  void setManagerName( const TQString& name );
  TQString managerName() const;

  void setAssistant( const TQString& name );
  TQString assistant() const;

  void setNickName( const TQString& name );
  TQString nickName() const;

  void setSpouseName( const TQString& name );
  TQString spouseName() const;

  void setBirthday( const TQDate& date );
  TQDate birthday() const;

  void setAnniversary( const TQDate& date );
  TQDate anniversary() const;

  void setPicture( const TQImage& image) { mPicture = image; }
  TQString pictureAttachmentName() const { return mPictureAttachmentName; }
  TQImage picture() const { return mPicture; }

  void setLogo( const TQImage& image ) { mLogo = image; }
  TQString logoAttachmentName() const { return mLogoAttachmentName; }
  TQImage logo() const { return mLogo; }

  void setSound( const TQByteArray& sound ) { mSound = sound; }
  TQString soundAttachmentName() const { return mSoundAttachmentName; }
  TQByteArray sound() const { return mSound; }

  void setChildren( const TQString& children );
  TQString children() const;

  void setGender( const TQString& gender );
  TQString gender() const;

  void setLanguage( const TQString& language );
  TQString language() const;

  void addPhoneNumber( const PhoneNumber& number );
  TQValueList<PhoneNumber>& phoneNumbers();
  const TQValueList<PhoneNumber>& phoneNumbers() const;

  void addEmail( const Email& email );
  TQValueList<Email>& emails();
  const TQValueList<Email>& emails() const;

  void addAddress( const Address& address );
  TQValueList<Address>& addresses();
  const TQValueList<Address>& addresses() const;

  // which address is preferred: home or business or other
  void setPreferredAddress( const TQString& address );
  TQString preferredAddress() const;

  float latitude() const { return mLatitude; }
  void setLatitude( float latitude ) { mLatitude = latitude; }

  float longitude() const { return mLongitude; }
  void setLongitude( float longitude ) { mLongitude = longitude; }

  // Load the attributes of this class
  bool loadAttribute( TQDomElement& );

  // Save the attributes of this class
  bool saveAttributes( TQDomElement& ) const;

  // Load this note by reading the XML file
  bool loadXML( const TQDomDocument& xml );

  // Serialize this note to an XML string
  TQString saveXML() const;

  // Return true if this contact is a distr list
  bool isDistributionList() const { return mIsDistributionList; }

protected:
  void setFields( const KABC::Addressee* );

private:
  bool loadNameAttribute( TQDomElement& element );
  void saveNameAttribute( TQDomElement& element ) const;

  bool loadPhoneAttribute( TQDomElement& element );
  void savePhoneAttributes( TQDomElement& element ) const;

  void saveEmailAttributes( TQDomElement& element ) const;

  bool loadAddressAttribute( TQDomElement& element );
  void saveAddressAttributes( TQDomElement& element ) const;

  void loadCustomAttributes( TQDomElement& element );
  void saveCustomAttributes( TQDomElement& element ) const;

  void loadDistrListMember( const TQDomElement& element );
  void saveDistrListMembers( TQDomElement& element ) const;

  TQImage loadPictureFromKMail( const TQString& attachmentName, KABC::ResourceKolab* resource, const TQString& subResource, Q_UINT32 sernum );
  TQImage loadPictureFromAddressee( const KABC::Picture& picture );

  TQByteArray loadDataFromKMail( const TQString& attachmentName, KABC::ResourceKolab* resource, const TQString& subResource, Q_UINT32 sernum );
  TQByteArray loadSoundFromAddressee( const KABC::Sound& sound );

  TQString productID() const;

  TQString mGivenName;
  TQString mMiddleNames;
  TQString mLastName;
  TQString mFullName;
  TQString mInitials;
  TQString mPrefix;
  TQString mSuffix;
  TQString mRole;
  TQString mFreeBusyUrl;
  TQString mOrganization;
  TQString mWebPage;
  TQString mIMAddress;
  TQString mDepartment;
  TQString mOfficeLocation;
  TQString mProfession;
  TQString mJobTitle;
  TQString mManagerName;
  TQString mAssistant;
  TQString mNickName;
  TQString mSpouseName;
  TQDate mBirthday;
  TQDate mAnniversary;
  TQImage mPicture;
  TQImage mLogo;
  TQByteArray mSound;
  TQString mPictureAttachmentName;
  TQString mLogoAttachmentName;
  TQString mSoundAttachmentName;
  TQString mChildren;
  TQString mGender;
  TQString mLanguage;
  TQValueList<PhoneNumber> mPhoneNumbers;
  TQValueList<Email> mEmails;
  TQValueList<Address> mAddresses;
  TQString mPreferredAddress;
  float mLatitude;
  float mLongitude;
  bool mHasGeo;
  bool mIsDistributionList;
  struct Custom {
    TQString app;
    TQString name;
    TQString value;
  };
  TQValueList<Custom> mCustomList;
  struct Member {
    TQString displayName;
    TQString email;
  };
  TQValueList<Member> mDistrListMembers;
};

}

#endif // KOLABCONTACT_H
