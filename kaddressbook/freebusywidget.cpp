/*
    This file is part of KAddressBook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include <QLabel>
#include <QLayout>
//Added by qt3to4:
#include <QHBoxLayout>

#include <kdialog.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <kcal/freebusyurlstore.h>

#include "freebusywidget.h"

FreeBusyWidget::FreeBusyWidget( KABC::AddressBook *ab, QWidget *parent )
  : KAB::ContactEditorWidget( ab, parent )
{
  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( KDialog::marginHint() );

  QLabel *label = new QLabel( i18n( "Location of Free/Busy information:" ), this );
  layout->addWidget( label );

  mURL = new KUrlRequester( this );
  label->setBuddy( mURL );
  layout->addWidget( mURL );

  connect( mURL, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( setModified() ) );
}

FreeBusyWidget::~FreeBusyWidget()
{
}

void FreeBusyWidget::loadContact( KABC::Addressee *addr )
{
  if ( addr->preferredEmail().isEmpty() )
    return;

  mURL->setUrl( KCal::FreeBusyUrlStore::self()->readUrl( addr->preferredEmail() ) );
}

void FreeBusyWidget::storeContact( KABC::Addressee *addr )
{
  if ( addr->preferredEmail().isEmpty() )
    return;

  KCal::FreeBusyUrlStore::self()->writeUrl( addr->preferredEmail(), mURL->url().toString() );
  KCal::FreeBusyUrlStore::self()->sync();
}

void FreeBusyWidget::setReadOnly( bool readOnly )
{
  mURL->setEnabled( !readOnly );
}

#include "freebusywidget.moc"
