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
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

CryptoFormatSelectionDialog::CryptoFormatSelectionDialog( QWidget *parent )
  : QDialog( parent )
{
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  mCryptoFormatCombo = new KComboBox;
  mainLayout->addWidget(mCryptoFormatCombo);
  mainLayout->addWidget(buttonBox);

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
