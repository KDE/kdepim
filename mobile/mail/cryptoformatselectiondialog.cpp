/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "cryptoformatselectiondialog.h"

#include <kcombobox.h>
#include <messagecomposer/utils/kleo_util.h>

CryptoFormatSelectionDialog::CryptoFormatSelectionDialog( QWidget *parent )
  : KDialog( parent )
{
  setButtons( Ok | Cancel );

  mCryptoFormatCombo = new KComboBox;
  setMainWidget( mCryptoFormatCombo );

  for ( int i = 0; i < numCryptoMessageFormats; ++i ) {
    if ( cryptoMessageFormats[ i ] != Kleo::InlineOpenPGPFormat ) // deprecated
      mCryptoFormatCombo->addItem( Kleo::cryptoMessageFormatToLabel( cryptoMessageFormats[ i ] ), static_cast<int>( cryptoMessageFormats[ i ] ) );
  }
}

CryptoFormatSelectionDialog::~CryptoFormatSelectionDialog()
{
}

void CryptoFormatSelectionDialog::setCryptoFormat( Kleo::CryptoMessageFormat format )
{
  mCryptoFormatCombo->setCurrentIndex( mCryptoFormatCombo->findData( static_cast<int>( format ) ) );
}

Kleo::CryptoMessageFormat CryptoFormatSelectionDialog::cryptoFormat() const
{
  return static_cast<Kleo::CryptoMessageFormat>( mCryptoFormatCombo->itemData( mCryptoFormatCombo->currentIndex() ).toInt() );
}
