/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "addaddresseedialog.h"

#include <QtGui/QLabel>
#include <QtGui/QTableWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QComboBox>
#include <QtCore/QRegExp>
#include <QtGui/QRegExpValidator>
#include <QtGui/QHeaderView>

#include <KLineEdit>
#include <KPushButton>
#include <KLocale>
#include <KIcon>
#include <KPassivePopup>

AddAddresseeDialog::AddAddresseeDialog( QWidget* parent )
 : KDialog( parent )
{
    setCaption( i18n( "New contact" ) );
    setButtons( KDialog::Ok | KDialog::Cancel );

    setupGui();
}


AddAddresseeDialog::~AddAddresseeDialog()
{
}

void AddAddresseeDialog::show() {
    // clean up any existing entries if necessarry
    if( isHidden() ) {
        m_email->clear();
        m_name->clear();
        m_phoneNumber->clear();
        m_phoneNumberTable->clear();

        // the table header must be set here since clear() also clears the header?!
        QStringList header;
        header << i18n( "Phone number" );
        header << i18n( "Number type" );
        m_phoneNumberTable->setHorizontalHeaderLabels( header );
    }

    KDialog::show();
    m_name->setFocus( Qt::OtherFocusReason );
}

QString AddAddresseeDialog::memorySlotToString( KMobileTools::AddressbookEntry::MemorySlot memorySlot ) {
    switch( memorySlot ) {
        case KMobileTools::AddressbookEntry::Phone:
            return i18n( "Phone internal memory" );

        case KMobileTools::AddressbookEntry::Sim:
            return i18n( "Sim card" );

        case KMobileTools::AddressbookEntry::DataCard:
            return i18n( "Memory card" );

        case KMobileTools::AddressbookEntry::Unknown:
            return i18n( "Unknown storage location" );
    }

    return QString();
}

void AddAddresseeDialog::availableSlots( KMobileTools::AddressbookEntry::MemorySlots memorySlots )
{
    m_storageLocation->clear();

    if( memorySlots.testFlag( KMobileTools::AddressbookEntry::Phone ) )
            m_storageLocation->addItem( memorySlotToString( KMobileTools::AddressbookEntry::Phone ),
                                        KMobileTools::AddressbookEntry::Phone );

    if( memorySlots.testFlag( KMobileTools::AddressbookEntry::Sim ) )
            m_storageLocation->addItem( memorySlotToString( KMobileTools::AddressbookEntry::Sim ),
                                        KMobileTools::AddressbookEntry::Sim );

    if( memorySlots.testFlag( KMobileTools::AddressbookEntry::DataCard ) )
            m_storageLocation->addItem( memorySlotToString( KMobileTools::AddressbookEntry::DataCard ),
                                        KMobileTools::AddressbookEntry::DataCard );

    if( memorySlots.testFlag( KMobileTools::AddressbookEntry::Unknown ) ) {
            m_storageLocation->addItem( memorySlotToString( KMobileTools::AddressbookEntry::Unknown ),
                                        KMobileTools::AddressbookEntry::Unknown );
    }

}

void AddAddresseeDialog::accept() {
    /// @TODO add validation here
    KMobileTools::AddressbookEntry entry;

    entry.setName( m_name->text() );
    entry.insertEmail( m_email->text() );

    // insert the phone numbers
    for( int i=0; i<m_phoneNumberTable->rowCount(); i++ ) {
        QTableWidgetItem* item;

        // process the phone number item
        item = m_phoneNumberTable->item( i, 0 );
        QString number = item->text();

        // process the phone number type item
        item = m_phoneNumberTable->item( i, 1 );
        QVariant typeVariant = item->data( Qt::UserRole );
        KABC::PhoneNumber::TypeFlag type = static_cast<KABC::PhoneNumber::TypeFlag>( typeVariant.toInt() );

        entry.insertPhoneNumber( KABC::PhoneNumber( number, type ) );
    }

    // set the memory slot
    QVariant memorySlot = m_storageLocation->itemData( m_storageLocation->currentIndex() );
    entry.setMemorySlot( static_cast<KMobileTools::AddressbookEntry::MemorySlot>( memorySlot.toInt() ) );

    emit addAddressee( entry );
    KDialog::accept();
}

void AddAddresseeDialog::addPhoneNumber() {
    QTableWidgetItem* phoneNumberItem = new QTableWidgetItem( m_phoneNumber->text() );
    QTableWidgetItem* typeItem = new QTableWidgetItem( m_phoneNumberTypes->currentText() );
    typeItem->setData( Qt::UserRole, m_phoneNumberTypes->itemData( m_phoneNumberTypes->currentIndex() ) );

    m_phoneNumberTable->insertRow( m_phoneNumberTable->rowCount() );
    int row = m_phoneNumberTable->rowCount() - 1;
    m_phoneNumberTable->setItem( row, 0, phoneNumberItem );
    m_phoneNumberTable->setItem( row, 1, typeItem );

    m_phoneNumberTable->resizeColumnsToContents();
}

void AddAddresseeDialog::removePhoneNumber() {
    QTableWidgetItem* currentItem = m_phoneNumberTable->currentItem();
    if( currentItem ) {
        int row = currentItem->row();
        m_phoneNumberTable->removeRow( row );
    }

    m_phoneNumberTable->resizeColumnsToContents();
}

void AddAddresseeDialog::setupGui() {
    m_widget = new QWidget( this );

    // name
    QLabel* nameLabel = new QLabel( i18n( "Name:" ), m_widget );
    m_name = new KLineEdit( m_widget );
    nameLabel->setBuddy( m_name );

    // e-mail
    QLabel* emailLabel = new QLabel( i18n( "E-mail:" ), m_widget );
    m_email = new KLineEdit( m_widget );

    QRegExp emailRegExp( "^[a-zA-Z][\\w\\.-]*[a-zA-Z0-9]@[a-zA-Z0-9]"
                         "[\\w\\.-]*[a-zA-Z0-9]\\.[a-zA-Z][a-zA-Z\\.]*[a-zA-Z]$" );
    QRegExpValidator* emailValidator = new QRegExpValidator( emailRegExp, 0 );
    m_email->setValidator( emailValidator );

    emailLabel->setBuddy( m_email );

    // phone numbers
    QLabel* phoneLabel = new QLabel( i18n( "Phone numbers:" ), m_widget );

    m_phoneNumber = new KLineEdit( m_widget );
    /// @TODO install a good reg exp here

    m_phoneNumberTable = new QTableWidget( m_widget );
    m_phoneNumberTable->setColumnCount( 2 );
    m_phoneNumberTable->setRowCount( 0 );
    m_phoneNumberTable->verticalHeader()->hide();
    m_phoneNumberTable->setShowGrid( false );
    m_phoneNumberTable->setSelectionBehavior( QAbstractItemView::SelectRows );
    m_phoneNumberTable->setSelectionMode( QAbstractItemView::SingleSelection );
    m_phoneNumberTable->setEditTriggers( QAbstractItemView::NoEditTriggers );

    m_phoneNumberTypes = new QComboBox( m_widget );
    m_phoneNumberTypes->addItem( i18n( "Mobile" ), KABC::PhoneNumber::Cell );
    m_phoneNumberTypes->addItem( i18n( "Home" ), KABC::PhoneNumber::Home );
    m_phoneNumberTypes->addItem( i18n( "Work" ), KABC::PhoneNumber::Work );
    m_phoneNumberTypes->addItem( i18n( "Messaging" ), KABC::PhoneNumber::Msg );
    m_phoneNumberTypes->addItem( i18n( "Voice" ), KABC::PhoneNumber::Voice );
    m_phoneNumberTypes->addItem( i18n( "Fax" ), KABC::PhoneNumber::Fax );
    m_phoneNumberTypes->addItem( i18n( "Pager" ), KABC::PhoneNumber::Pager );

    m_addPhoneNumber = new KPushButton( i18n( "Add phone number" ), m_widget );
    m_removePhoneNumber = new KPushButton( i18n( "Remove phone number" ), m_widget );
    connect( m_addPhoneNumber, SIGNAL(clicked()), this, SLOT(addPhoneNumber()) );
    connect( m_removePhoneNumber, SIGNAL(clicked()), this, SLOT(removePhoneNumber()) );

    // storage location (memory slot)
    QLabel* storageLocationLabel = new QLabel( i18n( "Storage location:" ), m_widget );
    m_storageLocation = new QComboBox( m_widget );

    QLabel* information = new QLabel( m_widget );
    KIcon informationIcon = KIcon( "dialog-information" );
    information->setPixmap( informationIcon.pixmap( 32, 32 ) );
    QLabel* noticeLabel = new QLabel( i18n( "Please note that not all kind of contact information "
                                            "might be supported by your phone." ) );

    QGridLayout* layout1 = new QGridLayout;
    layout1->setHorizontalSpacing( 5 );
    layout1->addWidget( nameLabel, 0, 0 );
    layout1->addWidget( m_name, 0, 1 );
    layout1->addWidget( emailLabel, 1, 0 );
    layout1->addWidget( m_email, 1, 1 );

    QGridLayout* layout2 = new QGridLayout;
    layout1->setHorizontalSpacing( 5 );
    layout2->addWidget( phoneLabel, 0, 0, 1, 2 );
    layout2->addWidget( m_phoneNumber, 1, 0 );
    layout2->addWidget( m_phoneNumberTypes, 1, 1 );
    layout2->addWidget( m_phoneNumberTable, 2, 0, 3, 1 );
    layout2->setRowStretch( 4, 1 );
    layout2->addWidget( m_addPhoneNumber, 2, 1 );
    layout2->addWidget( m_removePhoneNumber, 3, 1 );

    QGridLayout* layout3 = new QGridLayout;
    layout1->setHorizontalSpacing( 5 );
    layout3->addWidget( storageLocationLabel, 0, 0 );
    layout3->addWidget( m_storageLocation, 0, 1 );
    layout3->setColumnStretch( 1, 1 );

    QGridLayout* layout4 = new QGridLayout;
    layout1->setHorizontalSpacing( 5 );
    layout4->addWidget( information, 0, 0 );
    layout4->addWidget( noticeLabel, 0, 1 );
    layout4->setColumnStretch( 1, 1 );

    QVBoxLayout* main = new QVBoxLayout;
    main->addLayout( layout1, 0 );
    main->addSpacing( 1 );
    main->addLayout( layout2, 1 );
    main->addSpacing( 10 );
    main->addLayout( layout3, 0 );
    main->addSpacing( 10 );
    main->addLayout( layout4, 0 );

    m_widget->setLayout( main );

    setMainWidget( m_widget );

}

#include "addaddresseedialog.moc"
