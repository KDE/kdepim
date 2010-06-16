/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#include "editorbusiness.h"

#include "ui_editorbusiness.h"

#include <KABC/Addressee>

class EditorBusiness::Private
{
  EditorBusiness *const q;

  public:
    explicit Private( EditorBusiness *parent ) : q( parent )
    {
      mUi.setupUi( parent );
      mUi.logoButton->setIcon( KIcon( QLatin1String( "image-x-generic" ) ) );
    }

  public:
    Ui::EditorBusiness mUi;

    KABC::Addressee mContact;
};

static QString loadCustom( const KABC::Addressee &contact, const QString &key )
{
  return contact.custom( QLatin1String( "KADDRESSBOOK" ), key );
}

static void storeCustom( KABC::Addressee &contact, const QString &key, const QString &value )
{
  if ( value.isEmpty() )
    contact.removeCustom( QLatin1String( "KADDRESSBOOK" ), key );
  else
    contact.insertCustom( QLatin1String( "KADDRESSBOOK" ), key, value );
}


EditorBusiness::EditorBusiness( QWidget *parent )
  : EditorBase( parent ), d( new Private( this ) )
{
}

EditorBusiness::~EditorBusiness()
{
  delete d;
}

void EditorBusiness::loadContact( const KABC::Addressee &contact )
{
  if ( contact.logo().isEmpty() )
    d->mUi.logoButton->setIcon( KIcon( QLatin1String( "image-x-generic" ) ) );
  else
    d->mUi.logoButton->setIcon( QPixmap::fromImage( contact.logo().data() ) );

  d->mUi.organizationLineEdit->setText( contact.organization() );
  d->mUi.professionLineEdit->setText( loadCustom( contact, QLatin1String( "X-Profession" ) ) );
  d->mUi.titleLineEdit->setText( contact.title() );
  d->mUi.departmentLineEdit->setText( contact.department() );
  d->mUi.officeLineEdit->setText( loadCustom( contact, QLatin1String( "X-Office" ) ) );
  d->mUi.managerLineEdit->setText( loadCustom( contact, QLatin1String( "X-ManagersName" ) ) );
  d->mUi.assistantLineEdit->setText( loadCustom( contact, QLatin1String( "X-AssistantsName" ) ) );
}

void EditorBusiness::saveContact( KABC::Addressee &contact ) const
{
  contact.setOrganization( d->mUi.organizationLineEdit->text().trimmed() );
  storeCustom( contact, QLatin1String( "X-Profession" ), d->mUi.professionLineEdit->text().trimmed() );
  contact.setTitle( d->mUi.titleLineEdit->text().trimmed() );
  contact.setDepartment( d->mUi.departmentLineEdit->text().trimmed() );
  storeCustom( contact, QLatin1String( "X-Office" ), d->mUi.officeLineEdit->text().trimmed() );
  storeCustom( contact, QLatin1String( "X-ManagersName" ), d->mUi.managerLineEdit->text().trimmed() );
  storeCustom( contact, QLatin1String( "X-AssistantsName" ), d->mUi.assistantLineEdit->text().trimmed() );
}

#include "editorbusiness.moc"
