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

#include <KContacts/Addressee>

#include <QAbstractTextDocumentLayout>

class EditorBusiness::Private
{
  EditorBusiness *const q;

  public:
    explicit Private( EditorBusiness *parent ) : q( parent )
    {
      mUi.setupUi( parent );
      mUi.logoButton->setType( ImageWidget::Logo );

      q->connect( mUi.organizationLineEdit, SIGNAL(textChanged(QString)),
                  q, SIGNAL(organizationChanged(QString)) );
    }

  public:
    Ui::EditorBusiness mUi;

    KContacts::Addressee mContact;
};

static QString loadCustom( const KContacts::Addressee &contact, const QString &key )
{
  return contact.custom( QLatin1String( "KADDRESSBOOK" ), key );
}

static void storeCustom( KContacts::Addressee &contact, const QString &key, const QString &value )
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

void EditorBusiness::loadContact( const KContacts::Addressee &contact, const Akonadi::ContactMetaData& )
{
  d->mUi.logoButton->loadContact( contact );
  d->mUi.organizationLineEdit->setText( contact.organization() );
  d->mUi.professionLineEdit->setText( loadCustom( contact, QLatin1String( "X-Profession" ) ) );
  d->mUi.titleLineEdit->setText( contact.title() );
  d->mUi.departmentLineEdit->setText( contact.department() );
  d->mUi.officeLineEdit->setText( loadCustom( contact, QLatin1String( "X-Office" ) ) );
  d->mUi.managerLineEdit->setText( loadCustom( contact, QLatin1String( "X-ManagersName" ) ) );
  d->mUi.assistantLineEdit->setText( loadCustom( contact, QLatin1String( "X-AssistantsName" ) ) );
  d->mUi.noteTextEdit->setPlainText( contact.note() );

  d->mUi.noteTextEdit->setMinimumHeight( qMax( 200, (int)d->mUi.noteTextEdit->document()->documentLayout()->documentSize().height() + 50 ) );
}

void EditorBusiness::saveContact( KContacts::Addressee &contact, Akonadi::ContactMetaData& ) const
{
  d->mUi.logoButton->storeContact( contact );
  contact.setOrganization( d->mUi.organizationLineEdit->text().trimmed() );
  storeCustom( contact, QLatin1String( "X-Profession" ), d->mUi.professionLineEdit->text().trimmed() );
  contact.setTitle( d->mUi.titleLineEdit->text().trimmed() );
  contact.setDepartment( d->mUi.departmentLineEdit->text().trimmed() );
  storeCustom( contact, QLatin1String( "X-Office" ), d->mUi.officeLineEdit->text().trimmed() );
  storeCustom( contact, QLatin1String( "X-ManagersName" ), d->mUi.managerLineEdit->text().trimmed() );
  storeCustom( contact, QLatin1String( "X-AssistantsName" ), d->mUi.assistantLineEdit->text().trimmed() );
  contact.setNote( d->mUi.noteTextEdit->toPlainText() );
}

