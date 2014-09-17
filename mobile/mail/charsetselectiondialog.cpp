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

#include "charsetselectiondialog.h"

#include <messageviewer/viewer/nodehelper.h>

#include <KLocalizedString>
#include <kcombobox.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

CharsetSelectionDialog::CharsetSelectionDialog( QWidget *parent )
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

  mCharsetCombo = new KComboBox;
  mainLayout->addWidget(mCharsetCombo);
  mainLayout->addWidget(buttonBox);

  QStringList charsets = MessageViewer::NodeHelper::supportedEncodings( false );
  charsets.prepend( i18n( "Auto" ) );

  mCharsetCombo->addItems( charsets );
}

CharsetSelectionDialog::~CharsetSelectionDialog()
{
}

void CharsetSelectionDialog::setCharset( const QString &charset )
{
  for ( int i = 0; i < mCharsetCombo->count(); ++i ) {
    if ( charset ==MessageViewer::NodeHelper::encodingForName( mCharsetCombo->itemText( i ) ) ) {
      mCharsetCombo->setCurrentIndex( i );
      return;
    }
  }

  mCharsetCombo->setCurrentIndex( 0 );
}

QString CharsetSelectionDialog::charset() const
{
  if ( mCharsetCombo->currentIndex() == 0 )
    return QString();
  else
    return MessageViewer::NodeHelper::encodingForName( mCharsetCombo->currentText() );
}
