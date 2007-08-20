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

#include "fakeengine.h"

#include <QtGui/QVBoxLayout>
#include <QtCore/QMutexLocker>
#include <QtCore/QTimer>
#include <KDebug>

FakeEngine::FakeEngine( QObject *parent, const QString& deviceName )
 : EngineXP( parent, deviceName )
{
    QWidget* widget = new QWidget( 0 );
    QVBoxLayout* layout = new QVBoxLayout( widget );
    m_status = new QTextEdit();
    m_status->setReadOnly( true );
    layout->addWidget( m_status );

    widget->resize( 300, 300 );
    widget->setWindowTitle( "Fake engine output" );
    widget->show();

    status( "FakeEngine loaded." );

    m_statusInformationFetched = false;
    m_informationFetched = false;
    m_addressbookFetched = false;
}

void FakeEngine::status( const QString& statusInformation )
{
    QMutexLocker locker( &m_displayMutex );
    m_status->append( statusInformation );
}

FakeEngine::~FakeEngine()
{
}

void FakeEngine::connectDevice()
{
    status( QString( "Initialized device %1" ).arg( deviceName() ) );
    m_initialized = true;
    emit deviceConnected();
}

void FakeEngine::disconnectDevice()
{
    if( !m_initialized ) {
        status( QString( "Engine is not initialized yet." ) );
        emit deviceDisconnected();
        return;
    }

    m_initialized = false;
    status( QString( "Shut down succeeded ;-)" ));
    QTimer::singleShot( 5*1000, this, SIGNAL(deviceDisconnected()) );
}

int FakeEngine::signalStrength() const
{
    if( !m_statusInformationFetched )
        return -1;

    return 99;
}

int FakeEngine::charge() const
{
    if( !m_statusInformationFetched )
        return -1;

    return 70;
}

KMobileTools::Status::PowerSupplyType FakeEngine::powerSupplyType() const
{
    if( !m_statusInformationFetched )
        return KMobileTools::Status::Unknown;

    return KMobileTools::Status::ACAdaptor;
}

bool FakeEngine::ringing() const
{
    return false;
}

QString FakeEngine::networkName() const
{
    if( !m_informationFetched )
        return QString( "Unknown" );

    return "Vodafone";
}

QString FakeEngine::manufacturer() const
{
    if( !m_informationFetched )
        return QString( "Unknown" );

    return "Nokia";
}

KMobileTools::Information::Manufacturer FakeEngine::manufacturerID() const
{
    if( !m_informationFetched )
        return KMobileTools::Information::Unknown;

    return KMobileTools::Information::Nokia;
}

QString FakeEngine::model() const
{
    if( !m_informationFetched )
        return QString( "Unknown" );

    return "3650";
}

QString FakeEngine::imei() const
{
    if( !m_informationFetched )
        return QString( "Unknown" );

    return "123456789";
}

QString FakeEngine::revision() const
{
    if( !m_informationFetched )
        return QString( "Unknown" );

    return "V4.3";
}

void FakeEngine::fetchStatusInformation()
{
    if( !m_initialized ) {
        status( QString( "Engine is not initialized yet." ) );
        return;
    }

    if( !m_statusInformationFetched ) {
        m_statusInformationFetched = true;

        emit signalStrengthChanged( signalStrength() );
        emit chargeChanged( charge() );
        emit chargeTypeChanged( powerSupplyType() );
        emit phoneRinging( ringing() );
    }

    status( "Status information fetched." );
    emit statusInformationFetched();
}

void FakeEngine::fetchInformation()
{
    if( !m_initialized ) {
        status( QString( "Engine is not initialized yet." ) );
        return;
    }

    if( !m_informationFetched ) {
        m_informationFetched = true;

        emit networkNameChanged( networkName() );
    }

    status( "Mobile phone information fetched." );
    emit informationFetched();
}

void FakeEngine::populateAddressbook()
{
    // building sim contacts
    for( int i=1; i<=10; i++ ) {
        KMobileTools::AddressbookEntry addressee;
        addressee.setName( QString( "Sim contact %1" ).arg( QString::number( i ) ) );
        addressee.insertEmail( QString( "dummy@kmobiletools.org" ) );
        addressee.setMemorySlot( KMobileTools::AddressbookEntry::Sim );
        addAddressee( addressee );
    }

    // building phone contacts
    for( int i=1; i<=5; i++ ) {
        KMobileTools::AddressbookEntry addressee;
        addressee.setName( QString( "Phone contact %1" ).arg( QString::number( i ) ) );
        addressee.insertEmail( QString( "dummy@kmobiletools.org" ) );
        addressee.setMemorySlot( KMobileTools::AddressbookEntry::Phone );
        addAddressee( addressee );
    }
}

void FakeEngine::fetchAddressbook()
{
    if( !m_initialized ) {
        status( QString( "Engine is not initialized yet." ) );
        return;
    }

    // first fetch?
    if( !m_addressbookFetched )
        populateAddressbook();

    for( int i=0; i<m_addedAddressees.size(); i++ ) {
        m_addressbook.append( m_addedAddressees.at( i ) );
        emit addresseeAdded( m_addedAddressees.at( i ) );
    }
    m_addedAddressees.clear();

    for( int i=0; i<m_addressbook.size(); i++ ) {
        for( int j=0; j<m_removedAddressees.size(); j++ ) {
            if( m_addressbook.at( i ).uid() == m_removedAddressees.at( j ) ) {
                emit addresseeRemoved( m_addressbook.at( i ) );
                m_addressbook.removeAt( i );
            }
        }
    }
    m_removedAddressees.clear();

    m_addressbookFetched = true;
    status( "Address book fetched." );
    emit addressbookFetched();
}

KMobileTools::AddressbookEntry::MemorySlots FakeEngine::availableMemorySlots() const
{
    if( !m_addressbookFetched )
        return KMobileTools::AddressbookEntry::Unknown;

    return ( KMobileTools::AddressbookEntry::Phone | KMobileTools::AddressbookEntry::Sim );
}

KMobileTools::Addressbook FakeEngine::addressbook() const
{
    if( !m_addressbookFetched )
        return KMobileTools::Addressbook();

    return m_addressbook;
}

void FakeEngine::addAddressee( const KMobileTools::AddressbookEntry& addressee )
{
    QMutexLocker locker( &m_mutex );

    m_addedAddressees.append( addressee );

    status( QString( "Enqueued addressee \"%1\" to be added." ).arg( addressee.name() ) );
}

void FakeEngine::editAddressee( const KMobileTools::AddressbookEntry& oldAddressee,
                                const KMobileTools::AddressbookEntry& newAddressee )
{
    QMutexLocker locker( &m_mutex );

    m_addedAddressees.append( newAddressee );
    m_removedAddressees << oldAddressee.uid();

    status( QString( "Enqueued addressee \"%1\" to be edited." ).arg( newAddressee.name() ) );
}

void FakeEngine::removeAddressee( const KMobileTools::AddressbookEntry& addressee )
{
    QMutexLocker locker( &m_mutex );

    m_removedAddressees << addressee.uid();
    status( QString( "Enqueued addressee \"%1\" to be removed." ).arg( addressee.name() ) );
}

K_EXPORT_COMPONENT_FACTORY( libkmobiletools_fake, FakeEngineFactory )

FakeEngineFactory::FakeEngineFactory()
{
}

FakeEngineFactory::~FakeEngineFactory()
{
}

FakeEngine *FakeEngineFactory::createObject(QObject *parent, const char *classname, const QStringList& args)
{
    Q_UNUSED(classname)
    QString deviceName = args.at( 0 );
    return new FakeEngine( parent, deviceName );
}

#include "fakeengine.moc"
