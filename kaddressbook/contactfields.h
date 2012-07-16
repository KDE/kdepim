/*
  This file is part of KAddressBook.

  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef CONTACTFIELDS_H
#define CONTACTFIELDS_H

#include <KABC/Addressee>

class ContactFields
{
  public:

    /**
     * Describes the standard fields that are available for every contact
     */
    enum Field {
      Undefined = 0,

      FormattedName,
      Prefix,
      GivenName,
      AdditionalName,
      FamilyName,
      Suffix,
      NickName,

      Birthday,
      Anniversary,

      HomeAddressStreet,
      HomeAddressPostOfficeBox,
      HomeAddressLocality,
      HomeAddressRegion,
      HomeAddressPostalCode,
      HomeAddressCountry,
      HomeAddressLabel,

      BusinessAddressStreet,
      BusinessAddressPostOfficeBox,
      BusinessAddressLocality,
      BusinessAddressRegion,
      BusinessAddressPostalCode,
      BusinessAddressCountry,
      BusinessAddressLabel,

      HomePhone,
      BusinessPhone,
      MobilePhone,
      HomeFax,
      BusinessFax,
      CarPhone,
      Isdn,
      Pager,

      PreferredEmail,
      Email2,
      Email3,
      Email4,

      Mailer,
      Title,
      Role,
      Organization,
      Note,
      Homepage,

      BlogFeed,
      Profession,
      Office,
      Manager,
      Assistant,
      Spouse
    };

    /**
     * Defines a list of Field enums.
     */
    typedef QVector<Field> Fields;

    /**
     * Returns the i18n label for the @p field.
     */
    static QString label( Field field );

    /**
     * Returns a list of all available fields.
     */
    static Fields allFields();

    /**
     * Sets the @p value of the @p field for the @p contact.
     */
    static void setValue( Field field, const QString &value, KABC::Addressee &contact );

    /**
     * Returns the value for the @p field of the @p contact.
     */
    static QString value( Field field, const KABC::Addressee &contact );
};

#endif
