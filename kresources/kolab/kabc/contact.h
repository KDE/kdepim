/*
    This file is part of libkabc and/or kaddressbook.
    Copyright (c) 2002 - 2004 Klar�lvdalens Datakonsult AB
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

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

namespace KABC {
  class Address;
}

namespace Kolab {

class Contact : public KolabBase {
public:
  struct PhoneNumber {
  public:
    QString type;
    QString number;
  };

  struct Email {
  public:
    QString displayName;
    QString address;
  };

  struct Address {
  public:
    QString type;
    QString street;
    QString city;
    QString state;
    QString zip;
    QString country;
  };

  explicit Contact( KABC::Address* address = 0 );
  ~Contact();

  QString type() const { return "Contact"; }

  void setGivenName( const QString& name );
  QString givenName() const;

  void setMiddleNames( const QString& names );
  QString middleNames() const;

  void setLastName( const QString& name );
  QString lastName() const;

  void setFullName( const QString& name );
  QString fullName() const;

  void setInitials( const QString& initials );
  QString initials() const;

  void setPrefix( const QString& prefix );
  QString prefix() const;

  void setSuffix( const QString& suffix );
  QString suffix() const;

  void setRole( const QString& role );
  QString role() const;

  void setFreeBusyUrl( const QString& fbUrl );
  QString freebusyUrl() const;

  void setOrganization( const QString& organization );
  QString organization() const;

  void setWebPage( const QString& url );
  QString webPage() const;

  void setIMAddress( const QString& imAddress );
  QString imAddress() const;

  void setDepartment( const QString& department );
  QString department() const;

  void setOfficeLocation( const QString& location );
  QString officeLocation() const;

  void setProfession( const QString& profession );
  QString profession() const;

  void setJobTitle( const QString& title );
  QString jobTitle() const;

  void setManagerName( const QString& name );
  QString managerName() const;

  void setAssistant( const QString& name );
  QString assistant() const;

  void setNickName( const QString& name );
  QString nickName() const;

  void setSpouseName( const QString& name );
  QString spouseName() const;

  void setBirthday( const QDate& date );
  QDate birthday() const;

  void setAnniversary( const QDate& date );
  QDate anniversary() const;

#if 0
  // TODO. Probably a QPixmap
  void setPicture( const QString& name );
  QString picture() const;
#endif

  void setChildren( const QString& children );
  QString children() const;

  void setGender( const QString& gender );
  QString gender() const;

  void setLanguage( const QString& language );
  QString language() const;

  void addPhoneNumber( const PhoneNumber& number );
  QValueList<PhoneNumber>& phoneNumbers();
  const QValueList<PhoneNumber>& phoneNumbers() const;

  void addEmail( const Email& email );
  QValueList<Email>& emails();
  const QValueList<Email>& emails() const;

  void addAddress( const Address& address );
  QValueList<Address>& addresses();
  const QValueList<Address>& addresses() const;

  void setPreferredAddress( const QString& address );
  QString preferredAddress() const;

protected:
  void setFields( KABC::Address* );

private:
  QString mGivenName;
  QString mMiddleNames;
  QString mLastName;
  QString mFullName;
  QString mInitials;
  QString mPrefix;
  QString mSuffix;
  QString mRole;
  QString mFreeBusyUrl;
  QString mOrganization;
  QString mWebPage;
  QString mIMAddress;
  QString mDepartment;
  QString mOfficeLocation;
  QString mProfession;
  QString mJobTitle;
  QString mManagerName;
  QString mAssistant;
  QString mNickName;
  QString mSpouseName;
  QDate mBirthday;
  QDate mAnniversary;
#if 0
  QPixmap mPicture;
#endif
  QString mChildren;
  QString mGender;
  QString mLanguage;
  QValueList<PhoneNumber> mPhoneNumbers;
  QValueList<Email> mEmails;
  QValueList<Address> mAddresses;
  QString mPreferredAddress;
};

}

#endif // KOLABCONTACT_H
