/*
  This file is part of KAddressBook.
  Copyright (c) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>
                     Tobias Koenig <tokoe@kde.org>

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "stylepage.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

#include <KComboBox>
#include <KDialog>
#include <KLocale>

// helper method to sort contact fields by field label
static bool contactFieldsNameLesser( const ContactFields::Field &field,
                                     const ContactFields::Field &otherField )
{
  return ( QString::localeAwareCompare( ContactFields::label( field ),
                                        ContactFields::label( otherField ) ) < 0 );
}

StylePage::StylePage( QWidget *parent, const char *name )
  : QWidget( parent )
{
  setObjectName( QLatin1String(name) );
  initGUI();

  initFieldCombo();

  mSortTypeCombo->addItem( i18nc( "@item:inlistbox Ascending sort order", "Ascending" ) );
  mSortTypeCombo->addItem( i18nc( "@item:inlistbox Descending sort order", "Descending" ) );

  connect( mStyleCombo, SIGNAL(activated(int)), SIGNAL(styleChanged(int)) );
}

StylePage::~StylePage()
{
}

void StylePage::setPreview( const QPixmap &pixmap )
{
  if ( pixmap.isNull() ) {
    mPreview->setText( i18nc( "@label", "(No preview available.)" ) );
  } else {
    mPreview->setPixmap( pixmap );
  }
}

void StylePage::addStyleName( const QString &name )
{
  mStyleCombo->addItem( name );
}

void StylePage::clearStyleNames()
{
  mStyleCombo->clear();
}

void StylePage::setSortField( ContactFields::Field field )
{
  mFieldCombo->setCurrentIndex( mFields.indexOf( field ) );
}

void StylePage::setSortOrder( Qt::SortOrder sortOrder )
{
  if ( sortOrder == Qt::AscendingOrder ) {
    mSortTypeCombo->setCurrentIndex( 0 );
  } else {
    mSortTypeCombo->setCurrentIndex( 1 );
  }
}

ContactFields::Field StylePage::sortField() const
{
  if ( mFieldCombo->currentIndex() == -1 ) {
    return ContactFields::GivenName;
  }

  return mFields[ mFieldCombo->currentIndex() ];
}

Qt::SortOrder StylePage::sortOrder() const
{
  return ( mSortTypeCombo->currentIndex() == 0 ? Qt::AscendingOrder : Qt::DescendingOrder );
}

void StylePage::initFieldCombo()
{
  mFieldCombo->clear();

  mFields = ContactFields::allFields();
  mFields.remove( 0 ); // remove ContactFields::Undefined

  qSort( mFields.begin(), mFields.end(), contactFieldsNameLesser );

  ContactFields::Fields::ConstIterator it;
  for ( it = mFields.constBegin(); it != mFields.constEnd(); ++it ) {
    mFieldCombo->addItem( ContactFields::label( *it ) );
  }
}

void StylePage::initGUI()
{
  setWindowTitle( i18nc( "@title:window", "Choose Printing Style" ) );

  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( KDialog::marginHint() );

  QLabel *label =
    new QLabel(
      i18nc( "@label:textbox",
             "What should the print look like?\n"
             "KAddressBook has several printing styles, designed for different purposes.\n"
             "Choose the style that suits your needs below." ), this );
  topLayout->addWidget( label, 0, 0, 1, 2 );

  QGroupBox *group = new QGroupBox( i18nc( "@title:group", "Sorting" ), this );
  QGridLayout *sortLayout = new QGridLayout();
  group->setLayout( sortLayout );
  sortLayout->setSpacing( KDialog::spacingHint() );
  sortLayout->setMargin( KDialog::marginHint() );
  sortLayout->setAlignment( Qt::AlignTop );

  label = new QLabel( i18nc( "@label:listbox", "Criterion:" ), group );
  sortLayout->addWidget( label, 0, 0 );

  mFieldCombo = new KComboBox( false, group );
  mFieldCombo->setToolTip(
    i18nc( "@info:tooltip", "Select the primary sort field" ) );
  mFieldCombo->setWhatsThis(
    i18nc( "@info:whatsthis",
           "From this list you can select the field on which your contacts are sorted "
           "in the print output.  Use the sorting order option to determine if the "
           "sort will be in ascending or descending order." ) );
  sortLayout->addWidget( mFieldCombo, 0, 1 );

  label = new QLabel( i18nc( "@label:listbox", "Order:" ), group );
  sortLayout->addWidget( label, 1, 0 );

  mSortTypeCombo = new KComboBox( false, group );
  mSortTypeCombo->setToolTip(
    i18nc( "@info:tooltip", "Select the sorting order" ) );
  mSortTypeCombo->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Choose if you want to sort your contacts in ascending or descending order. "
           "Use the sorting criterion option to specify on which contact field the sorting "
           "will be performed." ) );
  sortLayout->addWidget( mSortTypeCombo, 1, 1 );

  topLayout->addWidget( group, 1, 0 );

  group = new QGroupBox( i18nc( "@title:group", "Print Style" ), this );
  QVBoxLayout *styleLayout = new QVBoxLayout();
  group->setLayout( styleLayout );
  styleLayout->setSpacing( KDialog::spacingHint() );
  styleLayout->setMargin( KDialog::marginHint() );

  mStyleCombo = new KComboBox( false, group );
  mStyleCombo->setToolTip(
    i18nc( "@info:tooltip", "Select the print style" ) );
  mStyleCombo->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Choose your desired printing style. See the preview image to help you decide." ) );

  styleLayout->addWidget( mStyleCombo );

  mPreview = new QLabel( group );
  QFont font( mPreview->font() );
  font.setPointSize( 20 );
  mPreview->setFont( font );
  mPreview->setScaledContents( true );
  mPreview->setAlignment( Qt::AlignCenter );
  mPreview->setWordWrap( true );
  styleLayout->addWidget( mPreview );

  topLayout->addWidget( group, 1, 1 );
  topLayout->setRowStretch( 1, 1 );
}

int StylePage::printingStyle() const
{
  return mStyleCombo->currentIndex();
}

void StylePage::setPrintingStyle( int index )
{
  mStyleCombo->setCurrentIndex( index );
}

#include "stylepage.moc"
