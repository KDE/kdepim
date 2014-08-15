/*
    This file is part of Akonadi Contact.

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


#include "namepartseditwidget.h"

#include <QFormLayout>

#include <kcombobox.h>
#include <klineedit.h>
#include <KLocalizedString>
#include <qplatformdefs.h>

NamePartsEditWidget::NamePartsEditWidget( QWidget *parent )
  : QWidget( parent)
{
  QFormLayout *layout = new QFormLayout( this );
  layout->setLabelAlignment( Qt::AlignLeft );

  mPrefixCombo = new KComboBox( this );
  mPrefixCombo->setDuplicatesEnabled( false );
  mPrefixCombo->setEditable( true );

  mGivenNameEdit = new KLineEdit( this );

  mAdditionalNameEdit = new KLineEdit( this );

  mFamilyNameEdit = new KLineEdit( this );

  mSuffixCombo = new KComboBox( this );
  mSuffixCombo->setDuplicatesEnabled( false );
  mSuffixCombo->setEditable( true );

  layout->addRow( i18n( "Honorific prefixes:" ), mPrefixCombo );
  layout->addRow( i18n( "Given name:" ), mGivenNameEdit );
  layout->addRow( i18n( "Additional names:" ), mAdditionalNameEdit );
  layout->addRow( i18n( "Family names:" ), mFamilyNameEdit );
  layout->addRow( i18n( "Honorific suffixes:" ), mSuffixCombo );

  QStringList prefixList;
  prefixList += QString();
  prefixList += i18n( "Dr." );
  prefixList += i18n( "Miss" );
  prefixList += i18n( "Mr." );
  prefixList += i18n( "Mrs." );
  prefixList += i18n( "Ms." );
  prefixList += i18n( "Prof." );
  prefixList.sort();

  QStringList suffixList;
  suffixList += QString();
  suffixList += i18n( "I" );
  suffixList += i18n( "II" );
  suffixList += i18n( "III" );
  suffixList += i18n( "Jr." );
  suffixList += i18n( "Sr." );
  suffixList.sort();

  mPrefixCombo->addItems( prefixList );
  mSuffixCombo->addItems( suffixList );
  mPrefixCombo->lineEdit()->setFocus();

  connect( mSuffixCombo, SIGNAL(activated(int)), this, SLOT(inputChanged()) );
  connect( mSuffixCombo, SIGNAL(editTextChanged(QString)), this, SLOT(inputChanged()) );
  connect( mPrefixCombo, SIGNAL(activated(int)), this, SLOT(inputChanged()) );
  connect( mPrefixCombo, SIGNAL(editTextChanged(QString)), this, SLOT(inputChanged()) );
  connect( mFamilyNameEdit, SIGNAL(textChanged(QString)), this, SLOT(inputChanged()) );
  connect( mGivenNameEdit, SIGNAL(textChanged(QString)), this, SLOT(inputChanged()) );
  connect( mAdditionalNameEdit, SIGNAL(textChanged(QString)), this, SLOT(inputChanged()) );
}

void NamePartsEditWidget::loadContact( const KABC::Addressee &contact )
{
  mContact = contact;

  disconnect( mSuffixCombo, SIGNAL(activated(int)), this, SLOT(inputChanged()) );
  disconnect( mSuffixCombo, SIGNAL(editTextChanged(QString)), this, SLOT(inputChanged()) );
  disconnect( mPrefixCombo, SIGNAL(activated(int)), this, SLOT(inputChanged()) );
  disconnect( mPrefixCombo, SIGNAL(editTextChanged(QString)), this, SLOT(inputChanged()) );
  disconnect( mFamilyNameEdit, SIGNAL(textChanged(QString)), this, SLOT(inputChanged()) );
  disconnect( mGivenNameEdit, SIGNAL(textChanged(QString)), this, SLOT(inputChanged()) );
  disconnect( mAdditionalNameEdit, SIGNAL(textChanged(QString)), this, SLOT(inputChanged()) );

  mPrefixCombo->setItemText( mPrefixCombo->currentIndex(), mContact.prefix() );
  mGivenNameEdit->setText( mContact.givenName() );
  mAdditionalNameEdit->setText( mContact.additionalName() );
  mFamilyNameEdit->setText( mContact.familyName() );
  mSuffixCombo->setItemText( mSuffixCombo->currentIndex(), mContact.suffix() );

  connect( mSuffixCombo, SIGNAL(activated(int)), this, SLOT(inputChanged()) );
  connect( mSuffixCombo, SIGNAL(editTextChanged(QString)), this, SLOT(inputChanged()) );
  connect( mPrefixCombo, SIGNAL(activated(int)), this, SLOT(inputChanged()) );
  connect( mPrefixCombo, SIGNAL(editTextChanged(QString)), this, SLOT(inputChanged()) );
  connect( mFamilyNameEdit, SIGNAL(textChanged(QString)), this, SLOT(inputChanged()) );
  connect( mGivenNameEdit, SIGNAL(textChanged(QString)), this, SLOT(inputChanged()) );
  connect( mAdditionalNameEdit, SIGNAL(textChanged(QString)), this, SLOT(inputChanged()) );
}

void NamePartsEditWidget::storeContact( KABC::Addressee &contact ) const
{
  contact.setPrefix( mPrefixCombo->currentText() );
  contact.setGivenName( mGivenNameEdit->text() );
  contact.setAdditionalName( mAdditionalNameEdit->text() );
  contact.setFamilyName( mFamilyNameEdit->text() );
  contact.setSuffix( mSuffixCombo->currentText() );
}

void NamePartsEditWidget::inputChanged()
{
  storeContact( mContact );
  emit nameChanged( mContact );
}
