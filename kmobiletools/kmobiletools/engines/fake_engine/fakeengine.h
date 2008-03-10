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

#ifndef KMOBILETOOLSFAKEENGINE_H
#define KMOBILETOOLSFAKEENGINE_H

#include <QtCore/QObject>
#include <QtGui/QTextEdit>
#include <QtCore/QMutex>
#include <QtCore/QStringList>
#include <KLibFactory>

#include <libkmobiletools/enginexp.h>
#include <libkmobiletools/contactslist.h>
#include <libkmobiletools/jobxp.h>

#include <libkmobiletools/ifaces/status.h>
#include <libkmobiletools/ifaces/information.h>
#include <libkmobiletools/ifaces/addressbook.h>
#include <libkmobiletools/ifaces/wizardprovider.h>
#include <libkmobiletools/ifaces/jobprovider.h>

/**
    @author Matthias Lechner <matthias@lmme.de>
*/
class FakeEngine : public KMobileTools::EngineXP, // base class
                   public KMobileTools::Ifaces::Status,         // interfaces
                   public KMobileTools::Ifaces::Information,
                   public KMobileTools::Ifaces::Addressbook,
                   public KMobileTools::Ifaces::WizardProvider,
                   public KMobileTools::Ifaces::JobProvider
{
    Q_OBJECT
    Q_INTERFACES(KMobileTools::Ifaces::Status KMobileTools::Ifaces::Information)
    Q_INTERFACES(KMobileTools::Ifaces::Addressbook KMobileTools::Ifaces::WizardProvider)
    Q_INTERFACES(KMobileTools::Ifaces::JobProvider)

public:
    FakeEngine( QObject *parent, const QString& deviceName );
    virtual ~FakeEngine();

    //
    // Status interface implementation
    //
    int signalStrength() const;
    int charge() const;
    KMobileTools::Status::PowerSupplyType powerSupplyType() const;
    bool ringing() const;

    //
    // Information interface implementation
    //
    QString networkName() const;
    QString manufacturer() const;
    KMobileTools::Information::Manufacturer manufacturerID() const;
    QString model() const;
    QString imei() const;
    QString revision() const;

    //
    // Addressbook interface implementation
    //
    KMobileTools::AddressbookEntry::MemorySlots availableMemorySlots() const;
    KMobileTools::Addressbook addressbook() const;

    //
    // WizardProvider interface implementation
    //
    QList<QWizardPage*> pageList() const;

public Q_SLOTS:
    void connectDevice();
    void disconnectDevice();

    //
    // Status interface implementation
    //
    void fetchStatusInformation();

    //
    // Information interface implementation
    //
    void fetchInformation();

    //
    // Addressbook interface implementation
    //
    void fetchAddressbook();

    void addAddressee( const KMobileTools::AddressbookEntry& addressee );
    void editAddressee( const KMobileTools::AddressbookEntry& oldAddressee,
                        const KMobileTools::AddressbookEntry& newAddressee );
    void removeAddressee( const KMobileTools::AddressbookEntry& addressee );

Q_SIGNALS:
    //
    // Status interface implementation
    //
    void statusInformationFetched();
    void signalStrengthChanged( int );
    void chargeChanged( int );
    void chargeTypeChanged( KMobileTools::Status::PowerSupplyType );
    void phoneRinging( bool );

    //
    // Information interface implementation
    //
    void informationFetched();
    void networkNameChanged( const QString& );

    //
    // Addressbook interface implementation
    //
    void addressbookFetched();
    void addresseeAdded( const KMobileTools::AddressbookEntry& addressee );

    void addresseeEdited( const KMobileTools::AddressbookEntry& oldAddressee,
                          const KMobileTools::AddressbookEntry& newAddressee );

    void addresseeRemoved( const KMobileTools::AddressbookEntry& addressee );

    //
    // JobProvider interface implementation
    //
    void jobCreated( KMobileTools::JobXP* job );

private:
    void populateAddressbook();

    bool m_initialized;

    bool m_statusInformationFetched;
    bool m_informationFetched;
    bool m_addressbookFetched;

    QWidget* m_widget;
    QTextEdit* m_status;
    void status( const QString& statusInformation );

    QMutex m_displayMutex;
    QMutex m_mutex;

    KMobileTools::Addressbook m_addressbook;

    KMobileTools::Addressbook m_addedAddressees;
    QStringList m_removedAddressees;

};

class FakeEngineFactory : public KPluginFactory
{
   Q_OBJECT
public:
    FakeEngineFactory();
    virtual ~FakeEngineFactory();

    vitual QObject *create(
        const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword );
};

#endif
