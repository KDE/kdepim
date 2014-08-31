/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>
    Copyright (c) 2012 Laurent Montel <montel@kde.org>

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

#include "delegateselector.h"

#include <libkdepim/addressline/addresseelineedit.h>

#include <QHBoxLayout>
#include <KLocalizedString>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

DelegateSelector::DelegateSelector(QWidget * parent)
  : QDialog( parent )
{
  setWindowTitle( i18n("Select delegate") );
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mOkButton = buttonBox->button(QDialogButtonBox::Ok);
  mOkButton->setDefault(true);
  mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &DelegateSelector::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &DelegateSelector::reject);
  mOkButton->setDefault(true);

  QWidget *delegateBox = new QWidget(this);
  QHBoxLayout *delegateBoxHBoxLayout = new QHBoxLayout(delegateBox);
  delegateBoxHBoxLayout->setMargin(0);
  new QLabel( i18n("Delegate:"), delegateBox );
  mDelegate = new KPIM::AddresseeLineEdit( delegateBox );
  connect(mDelegate, &KPIM::AddresseeLineEdit::textChanged, this, &DelegateSelector::slotTextChanged);
  mRsvp = new QCheckBox( i18n("Keep me informed about status changes of this incidence."), this );
  mRsvp->setChecked( true );

  mainLayout->addWidget( delegateBox );
  mainLayout->addWidget( mRsvp );
  mainLayout->addWidget(buttonBox);

  mOkButton->setEnabled( false );
}

void DelegateSelector::slotTextChanged( const QString& text )
{
  mOkButton->setEnabled( !text.isEmpty() );
}

QString DelegateSelector::delegate() const
{
  return mDelegate->text();
}

bool DelegateSelector::rsvp() const
{
  return mRsvp->isChecked();
}

