/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include <libkdepim/addresseelineedit.h>

#include <khbox.h>
#include <klocale.h>

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

  KHBox *delegateBox = new KHBox( mainWidget() );
  new QLabel( i18n("Delegate:"), delegateBox );
  mDelegate = new KPIM::AddresseeLineEdit( delegateBox );

  mRsvp = new QCheckBox( i18n("Keep me informed about status changes of this incidence."), mainWidget() );
  mRsvp->setChecked( true );

  layout->addWidget( delegateBox );
  layout->addWidget( mRsvp );
}

QString DelegateSelector::delegate() const
{
  return mDelegate->text();
}

bool DelegateSelector::rsvp() const
{
  return mRsvp->isChecked();
}

#include "delegateselector.moc"
