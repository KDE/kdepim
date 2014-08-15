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

DelegateSelector::DelegateSelector(QWidget * parent)
  : KDialog( parent )
{
  setCaption( i18n("Select delegate") );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );

  QVBoxLayout *layout = new QVBoxLayout( mainWidget() );

  QWidget *delegateBox = new QWidget( mainWidget() );
  QHBoxLayout *delegateBoxHBoxLayout = new QHBoxLayout(delegateBox);
  delegateBoxHBoxLayout->setMargin(0);
  new QLabel( i18n("Delegate:"), delegateBox );
  mDelegate = new KPIM::AddresseeLineEdit( delegateBox );
  connect( mDelegate, SIGNAL(textChanged(QString)), SLOT(slotTextChanged(QString)) );
  mRsvp = new QCheckBox( i18n("Keep me informed about status changes of this incidence."), mainWidget() );
  mRsvp->setChecked( true );

  layout->addWidget( delegateBox );
  layout->addWidget( mRsvp );
  enableButtonOk( false );
}

void DelegateSelector::slotTextChanged( const QString& text )
{
  enableButtonOk( !text.isEmpty() );
}

QString DelegateSelector::delegate() const
{
  return mDelegate->text();
}

bool DelegateSelector::rsvp() const
{
  return mRsvp->isChecked();
}

