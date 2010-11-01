/*
  Copyright (c) 2001 Marc Mutz <mutz@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "emailaddressrequester.h"

#include <akonadi/contact/emailaddressselectiondialog.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>

#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QTreeView>

using namespace MessageCore;

class MessageCore::EmailAddressRequester::Private
{
  public:
    Private( EmailAddressRequester *qq )
      : q( qq )
    {
    }

    void slotAddressBook();

    EmailAddressRequester *q;
    QPushButton* mButton;
    KLineEdit* mLineEdit;
};    

void EmailAddressRequester::Private::slotAddressBook()
{
  Akonadi::EmailAddressSelectionDialog dlg( q );
  dlg.view()->view()->setSelectionMode( QAbstractItemView::MultiSelection );
  if ( !dlg.exec() )
    return;

  QStringList addressList;
  foreach ( const Akonadi::EmailAddressSelection &selection, dlg.selectedAddresses() )
    addressList << selection.quotedEmail();

  QString text = mLineEdit->text().trimmed();

  if ( !text.isEmpty() ) {
    if ( !text.endsWith( QLatin1Char( ',' ) ) )
      text += QLatin1String( ", " );
    else
      text += QLatin1Char( ' ' );
  }

  mLineEdit->setText( text + addressList.join( QLatin1String( "," ) ) );
}

EmailAddressRequester::EmailAddressRequester( QWidget *parent )
  : QWidget( parent ),
    d( new Private( this ) )
{
  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setSpacing( 4 );
  layout->setMargin( 0 );

  d->mLineEdit = new KLineEdit( this );
  layout->addWidget( d->mLineEdit, 1 );

  d->mButton = new QPushButton( this );
  d->mButton->setIcon( KIcon( "help-contents" ) );
  d->mButton->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  d->mButton->setFixedHeight( d->mLineEdit->sizeHint().height() );
  d->mButton->setToolTip( i18n( "Open Address Book" ) );
  layout->addWidget( d->mButton );

  connect( d->mButton, SIGNAL( clicked() ), this, SLOT( slotAddressBook() ) );
  connect( d->mLineEdit, SIGNAL( textChanged( const QString& ) ),
           this, SIGNAL( textChanged() ) );
}

EmailAddressRequester::~EmailAddressRequester()
{
  delete d;
}

void EmailAddressRequester::clear()
{
  d->mLineEdit->clear();
}

void EmailAddressRequester::setText( const QString &text )
{
  d->mLineEdit->setText( text );
}

QString EmailAddressRequester::text() const
{
  return d->mLineEdit->text();
}

KLineEdit* EmailAddressRequester::lineEdit() const
{
  return d->mLineEdit;
}

#include "emailaddressrequester.moc"
