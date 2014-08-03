/*
    Copyright (c) 2004 Klarälvdalens Datakonsult AB
                  2009 Tobias Koenig <tokoe@kde.org>

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

#ifndef CRYPTOPAGEPLUGIN_H
#define CRYPTOPAGEPLUGIN_H

#include <Akonadi/Contact/ContactEditorPagePlugin>

namespace Kleo {
class KeyRequester;
}

class KComboBox;
class QCheckBox;

using namespace Akonadi;

class CryptoPagePlugin : public Akonadi::ContactEditorPagePlugin
{
  Q_OBJECT
  Q_INTERFACES( Akonadi::ContactEditorPagePlugin )
  Q_PLUGIN_METADATA(IID "org.kde.kaddressbook.CryptoPagePlugin");
  public:
    CryptoPagePlugin();
    ~CryptoPagePlugin();

    virtual QString title() const;
    virtual void loadContact( const KABC::Addressee &contact );
    virtual void storeContact( KABC::Addressee &contact ) const;
    virtual void setReadOnly( bool readOnly );

  private:
    enum { NumberOfProtocols = 4 };
    QCheckBox* mProtocolCB[NumberOfProtocols];
    KComboBox* mSignPref;
    KComboBox* mCryptPref;
    Kleo::KeyRequester* mPgpKey;
    Kleo::KeyRequester* mSmimeCert;
    bool mReadOnly;
};

#endif
