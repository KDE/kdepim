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

#include "editorlocation.h"

#include "locationmodel.h"
#include "ui_editorlocation.h"

#include <KABC/kabc/Addressee>
#include <KMessageBox>

#include <KLocalizedString>
#include <KDialog>

#include <QCheckBox>
#include <QDataWidgetMapper>
#include <QGroupBox>

class AddressTypeDialog : public KDialog
{
  public:
    AddressTypeDialog( QWidget *parent = 0 )
      : KDialog( parent)
    {
      setWindowTitle( i18nc( "street/postal", "New Address" ) );

      QWidget *page = new QWidget(this);
      setMainWidget( page );
      QVBoxLayout *layout = new QVBoxLayout( page );
      layout->setSpacing( KDialog::spacingHint() );
      layout->setMargin( 0 );

      QGroupBox *box  = new QGroupBox( i18nc( "street/postal", "Address Types" ), page );
      layout->addWidget( box );
      mGroup = new QButtonGroup( box );
      mGroup->setExclusive ( false );

      QGridLayout *buttonLayout = new QGridLayout( box );

      mTypeList = KABC::Address::typeList();
      mTypeList.removeAll( KABC::Address::Pref );

      KABC::Address::TypeList::ConstIterator it;
      int i = 0;
      int row = 0;
      for ( it = mTypeList.constBegin(); it != mTypeList.constEnd(); ++it, ++i ) {
        QCheckBox *checkBox = new QCheckBox( KABC::Address::typeLabel( *it ), box );
        buttonLayout->addWidget( checkBox, row, i % 3 );

        if ( i % 3 == 2 )
            ++row;

        mGroup->addButton( checkBox );
      }
    }

    KABC::Address::Type type() const
    {
      KABC::Address::Type type;
      for ( int i = 0; i < mGroup->buttons().count(); ++i ) {
        QCheckBox *box = dynamic_cast<QCheckBox*>( mGroup->buttons().at( i ) );
        if ( box && box->isChecked() )
          type |= mTypeList[ i ];
      }

      return type;
    }

  private:
    QButtonGroup *mGroup;

    KABC::Address::TypeList mTypeList;
};


class EditorLocation::Private
{
  EditorLocation *const q;

  public:
    explicit Private( EditorLocation *parent ) : q( parent )
    {
      mUi.setupUi( parent );
      mModel = new LocationModel( q );

      mUi.addressSelectionCombo->setModel( mModel );
      mUi.addressSelectionCombo->setModelColumn( 0 );

      mMapper = new QDataWidgetMapper( q );
      mMapper->setModel( mModel );
      mMapper->addMapping( mUi.streetLineEdit, 1, "plainText" );
      mMapper->addMapping( mUi.postOfficeBoxLineEdit, 2 );
      mMapper->addMapping( mUi.localityLineEdit, 3 );
      mMapper->addMapping( mUi.regionLineEdit, 4 );
      mMapper->addMapping( mUi.postalCodeLineEdit, 5 );
      mMapper->addMapping( mUi.countryLineEdit, 6 );
      mMapper->addMapping( mUi.editLabelLineEdit, 7 );
      mMapper->toFirst();

      q->connect( mUi.addressSelectionCombo, SIGNAL(activated(int)),
                  mMapper, SLOT(setCurrentIndex(int)) );
      q->connect( mUi.addAddressButton, SIGNAL(clicked()),
                  SLOT(addAddress()) );
      q->connect( mUi.deleteAddressButton, SIGNAL(clicked()),
                  SLOT(removeAddress()) );
      q->connect( mModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                  SLOT(addressCountChanged()) );
      q->connect( mModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                  SLOT(addressCountChanged()) );

      addressCountChanged();
    }

    void addAddress()
    {
      AddressTypeDialog dlg;
      if ( !dlg.exec() )
        return;

      const KABC::Address::Type addressType = dlg.type();

      if ( mModel->insertRows( 0, 1 ) ) {
        mModel->setData( mModel->index( 0, 0 ), QVariant::fromValue( static_cast<int>( addressType ) ) );
        mUi.addressSelectionCombo->setCurrentIndex( 0 );
        mMapper->setCurrentIndex( 0 );
      }
    }

    void removeAddress()
    {
      const int answer = KMessageBox::questionYesNo( 0, i18n( "Do you really want to delete this address?" ),
                                                        i18n( "Delete Address" ),
                                                        KGuiItem( i18n("Delete") ) );
      if ( answer == KMessageBox::No )
        return;

      const int index = mMapper->currentIndex();
      mModel->removeRows( index, 1 );
      if ( index >= mModel->rowCount() )
        mMapper->setCurrentIndex( mModel->rowCount() - 1 );
      else
        mMapper->setCurrentIndex( index );

      if ( mModel->rowCount() == 0 ) {
        // We have to cleanup the fields ourself in this case,
        // QDataWidgetMapper does not handle a non-existing index
        for ( int column = 1; column < 8; ++column ) {
          QLineEdit *lineEdit = qobject_cast<QLineEdit*>( mMapper->mappedWidgetAt( column ) );
          if ( lineEdit )
            lineEdit->clear();
          else {
            QTextEdit *textEdit = qobject_cast<QTextEdit*>( mMapper->mappedWidgetAt( column ) );
            if ( textEdit )
              textEdit->clear();
          }
        }
      }
    }

    void addressCountChanged()
    {
      const bool enabled = (mModel->rowCount() > 0);

      mUi.addressSelectionCombo->setEnabled( enabled );
      mUi.deleteAddressButton->setEnabled( enabled );
      for ( int column = 1; column < 8; ++column ) {
        QWidget *widget = mMapper->mappedWidgetAt( column );
        if ( widget )
          widget->setEnabled( enabled );
      }
    }

  public:
    Ui::EditorLocation mUi;

    KABC::Addressee mContact;
    LocationModel *mModel;
    QDataWidgetMapper *mMapper;
};


EditorLocation::EditorLocation( QWidget *parent )
  : EditorBase( parent ), d( new Private( this ) )
{
}

EditorLocation::~EditorLocation()
{
  delete d;
}

void EditorLocation::loadContact( const KABC::Addressee &contact, const Akonadi::ContactMetaData& )
{
  d->mModel->setLocations( contact.addresses() );
  d->mUi.addressSelectionCombo->setCurrentIndex( 0 );
  d->mMapper->setCurrentIndex( 0 );
  d->addressCountChanged();
}

void EditorLocation::saveContact( KABC::Addressee &contact, Akonadi::ContactMetaData& ) const
{
  const KABC::Address::List oldAddresses = contact.addresses();
  foreach ( const KABC::Address &oldAddress, oldAddresses )
    contact.removeAddress( oldAddress );

  foreach ( const KABC::Address &newAddress, d->mModel->locations() )
    contact.insertAddress( newAddress );
}

#include "moc_editorlocation.cpp"
