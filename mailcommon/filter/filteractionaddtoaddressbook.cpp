/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "filteractionaddtoaddressbook.h"

#include "pimcommon/widgets/minimumcombobox.h"

#include "libkdepim/job/addcontactjob.h"

#include <CollectionComboBox>
#include <KABC/Addressee>
#include <KLineEdit>
#include <KLocale>
#include <KPIMUtils/Email>

#include <QGridLayout>
#include <QLabel>

using namespace MailCommon;

FilterAction* FilterActionAddToAddressBook::newAction()
{
    return new FilterActionAddToAddressBook;
}

FilterActionAddToAddressBook::FilterActionAddToAddressBook( QObject *parent )
    : FilterActionWithStringList( QLatin1String("add to address book"), i18n( "Add to Address Book" ), parent ),
      mFromStr( i18nc( "Email sender", "From" ) ),
      mToStr( i18nc( "Email recipient", "To" ) ),
      mCCStr( i18n( "CC" ) ),
      mBCCStr( i18n( "BCC" ) ),
      mHeaderType( FromHeader ),
      mCollectionId( -1 ),
      mCategory( i18n( "KMail Filter" ) )
{
}

bool FilterActionAddToAddressBook::isEmpty() const
{
    return false;
}

FilterAction::ReturnCode FilterActionAddToAddressBook::process(ItemContext &context , bool) const
{
    const KMime::Message::Ptr msg = context.item().payload<KMime::Message::Ptr>();

    QString headerLine;
    switch ( mHeaderType ) {
    case FromHeader: headerLine = msg->from()->asUnicodeString(); break;
    case ToHeader: headerLine = msg->to()->asUnicodeString(); break;
    case CcHeader: headerLine = msg->cc()->asUnicodeString(); break;
    case BccHeader: headerLine = msg->bcc()->asUnicodeString(); break;
    }

    const QStringList emails = KPIMUtils::splitAddressList( headerLine );

    foreach ( const QString& singleEmail, emails ) {
        QString name, email;
        KABC::Addressee::parseEmailAddress( singleEmail, name, email );

        KABC::Addressee contact;
        contact.setNameFromString( name );
        contact.insertEmail( email, true );
        if ( !mCategory.isEmpty() )
            contact.insertCategory( mCategory );

        KPIM::AddContactJob *job = new KPIM::AddContactJob( contact, Akonadi::Collection( mCollectionId ) );
        job->showMessageBox(false);
        job->start();
    }

    return GoOn;
}

SearchRule::RequiredPart FilterActionAddToAddressBook::requiredPart() const
{
    return SearchRule::Envelope;
}


QWidget* FilterActionAddToAddressBook::createParamWidget( QWidget *parent ) const
{
    QWidget *widget = new QWidget( parent );
    QGridLayout *layout = new QGridLayout( widget );

    PimCommon::MinimumComboBox *headerCombo = new PimCommon::MinimumComboBox( widget );
    headerCombo->setObjectName( QLatin1String("HeaderComboBox") );
    layout->addWidget( headerCombo, 0, 0, 2, 1, Qt::AlignVCenter );

    QLabel *label = new QLabel( i18n( "with category" ), widget );
    layout->addWidget( label, 0, 1 );

    KLineEdit *categoryEdit = new KLineEdit( widget );
    categoryEdit->setObjectName( QLatin1String("CategoryEdit") );
    categoryEdit->setTrapReturnKey(true);
    layout->addWidget( categoryEdit, 0, 2 );

    label = new QLabel( i18n( "in address book" ), widget );
    layout->addWidget( label, 1, 1 );

    Akonadi::CollectionComboBox *collectionComboBox = new Akonadi::CollectionComboBox( widget );
    collectionComboBox->setMimeTypeFilter( QStringList() << KABC::Addressee::mimeType() );
    collectionComboBox->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );

    collectionComboBox->setObjectName( QLatin1String("AddressBookComboBox") );
    collectionComboBox->setToolTip( i18n( "<p>This defines the preferred address book.<br />"
                                          "If it is not accessible, the filter will fallback to the default address book.</p>" ) );
    layout->addWidget( collectionComboBox, 1, 2 );

    connect( categoryEdit, SIGNAL(textChanged(QString)),
             this, SIGNAL(filterActionModified()) );
    connect( headerCombo, SIGNAL(currentIndexChanged(int)),
             this, SIGNAL(filterActionModified()) );
    connect( collectionComboBox, SIGNAL(activated(int)),
             this, SIGNAL(filterActionModified()) );

    setParamWidgetValue( widget );

    return widget;
}

void FilterActionAddToAddressBook::setParamWidgetValue( QWidget *paramWidget ) const
{
    PimCommon::MinimumComboBox *headerCombo = paramWidget->findChild<PimCommon::MinimumComboBox*>( QLatin1String("HeaderComboBox") );
    Q_ASSERT( headerCombo );
    headerCombo->clear();
    headerCombo->addItem( mFromStr, FromHeader );
    headerCombo->addItem( mToStr, ToHeader );
    headerCombo->addItem( mCCStr, CcHeader );
    headerCombo->addItem( mBCCStr, BccHeader );

    headerCombo->setCurrentIndex( headerCombo->findData( mHeaderType ) );

    KLineEdit *categoryEdit = paramWidget->findChild<KLineEdit*>( QLatin1String("CategoryEdit") );
    Q_ASSERT( categoryEdit );
    categoryEdit->setText( mCategory );

    Akonadi::CollectionComboBox *collectionComboBox = paramWidget->findChild<Akonadi::CollectionComboBox*>( QLatin1String("AddressBookComboBox") );
    Q_ASSERT( collectionComboBox );
    collectionComboBox->setDefaultCollection( Akonadi::Collection( mCollectionId ) );
    collectionComboBox->setProperty( "collectionId", mCollectionId );
}

void FilterActionAddToAddressBook::applyParamWidgetValue( QWidget *paramWidget )
{
    const PimCommon::MinimumComboBox *headerCombo = paramWidget->findChild<PimCommon::MinimumComboBox*>( QLatin1String("HeaderComboBox") );
    Q_ASSERT( headerCombo );
    mHeaderType = static_cast<HeaderType>( headerCombo->itemData( headerCombo->currentIndex() ).toInt() );

    const KLineEdit *categoryEdit = paramWidget->findChild<KLineEdit*>( QLatin1String("CategoryEdit") );
    Q_ASSERT( categoryEdit );
    mCategory = categoryEdit->text();

    const Akonadi::CollectionComboBox *collectionComboBox = paramWidget->findChild<Akonadi::CollectionComboBox*>( QLatin1String("AddressBookComboBox") );
    Q_ASSERT( collectionComboBox );
    const Akonadi::Collection collection = collectionComboBox->currentCollection();

    // it might be that the model of collectionComboBox has not finished loading yet, so
    // we use the previously 'stored' value from the 'collectionId' property
    if ( collection.isValid() ) {
        mCollectionId = collection.id();
        connect( collectionComboBox, SIGNAL(currentIndexChanged(int)),
                 this, SIGNAL(filterActionModified()) );
    } else {
        const QVariant value = collectionComboBox->property( "collectionId" );
        if ( value.isValid() )
            mCollectionId = value.toLongLong();
    }
}

void FilterActionAddToAddressBook::clearParamWidget( QWidget *paramWidget ) const
{
    PimCommon::MinimumComboBox *headerCombo = paramWidget->findChild<PimCommon::MinimumComboBox*>( QLatin1String("HeaderComboBox") );
    Q_ASSERT( headerCombo );
    headerCombo->setCurrentIndex( 0 );

    KLineEdit *categoryEdit = paramWidget->findChild<KLineEdit*>( QLatin1String("CategoryEdit") );
    Q_ASSERT( categoryEdit );
    categoryEdit->setText( mCategory );
}

QString FilterActionAddToAddressBook::argsAsString() const
{
    QString result;

    switch ( mHeaderType ) {
    case FromHeader: result = QLatin1String( "From" ); break;
    case ToHeader: result = QLatin1String( "To" ); break;
    case CcHeader: result = QLatin1String( "CC" ); break;
    case BccHeader: result = QLatin1String( "BCC" ); break;
    }

    result += QLatin1Char( '\t' );
    result += QString::number( mCollectionId );
    result += QLatin1Char( '\t' );
    result += mCategory;

    return result;
}


void FilterActionAddToAddressBook::argsFromString( const QString &argsStr )
{
    const QStringList parts = argsStr.split( QLatin1Char( '\t' ), QString::KeepEmptyParts );
    const QString firstElement = parts[ 0 ];
    if ( firstElement == QLatin1String( "From" ) )
        mHeaderType = FromHeader;
    else if ( firstElement == QLatin1String( "To" ) )
        mHeaderType = ToHeader;
    else if ( firstElement == QLatin1String( "CC" ) )
        mHeaderType = CcHeader;
    else if ( firstElement == QLatin1String( "BCC" ) )
        mHeaderType = BccHeader;

    if ( parts.count() >= 2 )
        mCollectionId = parts[ 1 ].toLongLong();

    if ( parts.count() < 3 )
        mCategory.clear();
    else
        mCategory = parts[ 2 ];
}

