/*
    This file is part of KAddressBook.
    Copyright (c) 2004 Klar�lvdalens Datakonsult AB

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

#ifndef CRYPTOWIDGET_H
#define CRYPTOWIDGET_H

#include "contacteditorwidget.h"

class TQComboBox;

namespace Kleo {
  class KeyRequester;
}
class TQCheckBox;

class CryptoWidget : public KAB::ContactEditorWidget
{
  Q_OBJECT

  public:
    CryptoWidget( KABC::AddressBook *ab, TQWidget *parent, const char *name = 0 );
    ~CryptoWidget();

    void loadContact( KABC::Addressee *addr );
    void storeContact( KABC::Addressee *addr );
    void setReadOnly( bool readOnly );

  private:
    enum { NumberOfProtocols = 4 };
    TQCheckBox* mProtocolCB[NumberOfProtocols];
    TQComboBox* mSignPref;
    TQComboBox* mCryptPref;
    Kleo::KeyRequester* mPgpKey;
    Kleo::KeyRequester* mSmimeCert;
    bool mReadOnly;
};

class CryptoWidgetFactory : public KAB::ContactEditorWidgetFactory
{
  public:
    CryptoWidgetFactory();
    KAB::ContactEditorWidget *createWidget( KABC::AddressBook *ab, TQWidget *parent, const char *name )
    {
      return new CryptoWidget( ab, parent, name );
    }

    TQString pageTitle() const;
    TQString pageIdentifier() const;
};

#endif
