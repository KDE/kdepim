/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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

#ifndef NAMEEDITDIALOG_H
#define NAMEEDITDIALOG_H

#include <kdialogbase.h>
#include <kabc/addressee.h>

#include "addresseeconfig.h"

class QCheckBox;

class KLineEdit;
class KComboBox;

/**
  Editor dialog for name details, like given name, family name etc.
*/
class NameEditDialog : public KDialogBase
{
  Q_OBJECT

  public:
    enum FormattedNameType
    {
      CustomName,           // returned by @ref customFormattedName()
      SimpleName,           // form: givenName familyName
      FullName,             // form: prefix givenName additionalName familyName suffix
      ReverseNameWithComma, // form: familyName, givenName
      ReverseName,          // form: familyName givenName
      Organization          // the organization name
    };

    NameEditDialog( const KABC::Addressee &addr, int type,
                    bool readOnly, TQWidget *parent, const char *name = 0 );
    ~NameEditDialog();

    TQString familyName() const;
    TQString givenName() const;
    TQString prefix() const;
    TQString suffix() const;
    TQString additionalName() const;
    TQString customFormattedName() const;
    int formattedNameType() const;

    bool changed() const;

    static TQString formattedName( const KABC::Addressee &addr, int type );

  protected slots:
    void slotHelp();

  private slots:
    void parseBoxChanged( bool );
    void formattedNameTypeChanged();
    void formattedNameChanged( const TQString& );
    void typeChanged( int );
    void initTypeCombo();
    void modified();

  private:
    KComboBox *mSuffixCombo;
    KComboBox *mPrefixCombo;
    KComboBox *mFormattedNameCombo;
    KLineEdit *mFamilyNameEdit;
    KLineEdit *mGivenNameEdit;
    KLineEdit *mAdditionalNameEdit;
    KLineEdit *mFormattedNameEdit;
    TQCheckBox *mParseBox;

    AddresseeConfig mAddresseeConfig;
    KABC::Addressee mAddressee;
    TQString mCustomFormattedName;
    bool mChanged;
};

#endif
