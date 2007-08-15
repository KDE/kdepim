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

#include "addressbook.h"
#include "addressbookentryitem.h"

#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QGridLayout>
#include <QtGui/QAction>
#include <QtCore/QPoint>
#include <QtGui/QMenu>

#include <KLocale>
#include <KIcon>
#include <KMessageBox>

#include <libkmobiletools/ifaces/addressbook.h>
#include <libkmobiletools/ifaces/status.h>
#include <libkmobiletools/deviceloader.h>
#include <libkmobiletools/enginexp.h>

Addressbook::Addressbook( QObject* parent, const QString& deviceName )
: QObject( parent )
{
    m_deviceName = deviceName;

    m_widget = 0;
    m_addAddresseeDialog = 0;

    m_engine = KMobileTools::DeviceLoader::instance()->engine( m_deviceName );
    if( m_engine ) {
        m_addressbook = qobject_cast<KMobileTools::Ifaces::Addressbook*> ( m_engine );

        if( m_addressbook ) {
            setupWidget();
            setupActions();
        }
    }
}


Addressbook::~Addressbook()
{
}

QString Addressbook::name() const {
    return i18n( "Address book" );
}

KIcon Addressbook::icon() const {
    return KIcon( "kaddressbook" );
}

QWidget* Addressbook::widget() const {
    Q_ASSERT( m_widget );
    return m_widget;
}

QStringList Addressbook::requires() const {
    return QStringList( "Addressbook" );
}

void Addressbook::setupWidget() {
    m_widget = new QWidget();

    m_addresseeList = new QListWidget( m_widget );
    m_addresseeList->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( m_addresseeList, SIGNAL(customContextMenuRequested(const QPoint&)),
             this, SLOT(addresseeListContextMenu(const QPoint&)) );

    m_addresseeSearch = new KListWidgetSearchLine( m_widget, m_addresseeList );
    m_addresseeDetails = new KTextBrowser( m_widget );

    QGridLayout* layout = new QGridLayout( m_widget );
    layout->addWidget( m_addresseeSearch, 0, 0 );
    layout->addWidget( m_addresseeList, 1, 0 );
    layout->addWidget( m_addresseeDetails, 0, 1, 2, 1 );
}

void Addressbook::setupActions() {
    m_newContact = new QAction( KIcon( "add-user" ), i18n( "New contact" ), this );
    m_newContact->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ) );

    m_editContact = new QAction( KIcon( "edit-user" ), i18n( "Edit contact" ), this );
    m_editContact->setEnabled( false );

    m_deleteContact = new QAction( KIcon( "delete-user" ), i18n( "Delete contact" ), this );
    m_deleteContact->setShortcut( Qt::Key_Delete );
    m_deleteContact->setEnabled( false );

    QAction *separator = new QAction( this );
    separator->setSeparator( true );

    QAction *reloadContacts = new QAction( KIcon( "view-refresh" ), i18n( "Reload contacts" ), this );
    reloadContacts->setShortcut( Qt::Key_F5 );

    QAction *separator2 = new QAction( this );
    separator2->setSeparator( true );

    QAction *importContacts = new QAction( KIcon( "file-import" ), i18n( "Import contacts" ), this );

    QAction *exportContacts = new QAction( KIcon( "file-export" ), i18n( "Export contacts" ), this );


    // connect actions
    connect( reloadContacts, SIGNAL(triggered()), m_engine, SLOT(fetchAddressbook()) );
    connect( m_engine, SIGNAL(addressbookFetched()), this, SLOT(cleanUpItems()) );
    connect( m_newContact, SIGNAL(triggered()), this, SLOT(requestEntryAddition()) );
    connect( m_deleteContact, SIGNAL(triggered()), this, SLOT(requestEntryRemoval()) );
    connect( m_editContact, SIGNAL(triggered()), this, SLOT(requestEntryEditing()) );

    connect( m_addresseeList, SIGNAL(itemSelectionChanged()), this, SLOT(checkEnableActions()) );

    // add actions to the action collection
    m_actionList.append( m_newContact );
    m_actionList.append( m_editContact );
    m_actionList.append( m_deleteContact );
    m_actionList.append( separator );
    m_actionList.append( reloadContacts );
    m_actionList.append( separator2 );
    m_actionList.append( importContacts );
    m_actionList.append( exportContacts );

    // setup poll timer
    m_fetchTimer = new QTimer( this );
    m_fetchTimer->setInterval( 10 * 1000 );
    connect( m_fetchTimer, SIGNAL(timeout()), m_engine, SLOT(fetchAddressbook()) );

    m_fetchTimer->start();

    // setup engine connections
    connect( m_engine, SIGNAL(addresseeAdded(const KMobileTools::AddressbookEntry&)),
             this, SLOT(addEntry(const KMobileTools::AddressbookEntry&)) );
    connect( m_engine, SIGNAL(addresseeRemoved(const KMobileTools::AddressbookEntry&)),
             this, SLOT(removeEntry(const KMobileTools::AddressbookEntry&)) );
}

QList<QAction*> Addressbook::actionList() const {
    return m_actionList;
}

void Addressbook::addresseeListContextMenu( const QPoint& position ) {
    QMenu menu;

    // look if there's an item at the clicked position
    QListWidgetItem* item = m_addresseeList->itemAt( position );
    if( item ) {
        AddressbookEntryItem* entry = static_cast<AddressbookEntryItem*>( item );
        if( entry ) {
            if( entry->state() == AddressbookEntryItem::Default ) {
                menu.addAction( m_editContact );
                menu.addAction( m_deleteContact );
            }
        }
    } else
        menu.addAction( m_newContact );

    menu.exec( m_addresseeList->mapToGlobal( position ) );
}

void Addressbook::cleanUpItems() {
    m_mutex.lock();

    for( int i=0; i<m_pendingItems.size(); i++ ) {
        AddressbookEntryItem* item = static_cast<AddressbookEntryItem*>( m_pendingItems.at( i ) );
        if( item ) {
            if( item->state() == AddressbookEntryItem::AdditionRequested )
                delete item;
        }
    }

    /// @todo mh, this is not really thread-safe
    m_pendingItems.clear();

    m_mutex.unlock();
}

void Addressbook::addEntry( const KMobileTools::AddressbookEntry& entry ) {
    AddressbookEntryItem* item = new AddressbookEntryItem( m_addresseeList );
    item->setText( entry.name() );
    item->setIcon( KIcon( "user" ) );
    item->setAddressee( entry );
}

void Addressbook::removeEntry( const KMobileTools::AddressbookEntry& entry ) {
    QList<QListWidgetItem*> items = m_addresseeList->findItems( entry.name(), Qt::MatchExactly );
    for( int i=0; i<items.size(); i++ ) {
        AddressbookEntryItem* item = static_cast<AddressbookEntryItem*>( items.at( i ) );
        if( item ) {
            if( item->addressee() == entry )
                delete items.at( i );
        }
    }
}

void Addressbook::findAvailableSlots() {
    emit foundAvailableSlots( m_addressbook->availableMemorySlots() );
}

void Addressbook::requestEntryAddition( const KMobileTools::AddressbookEntry& entry ) {
    AddressbookEntryItem* item = new AddressbookEntryItem( m_addresseeList );
    item->setText( entry.name() );
    item->setIcon( KIcon( "user" ) );
    item->setAddressee( entry );
    item->setState( AddressbookEntryItem::AdditionRequested );

    m_mutex.lock();
    m_pendingItems.append( item );
    m_mutex.unlock();

    m_addressbook->addAddressee( entry );
}

void Addressbook::requestEntryAddition() {
    if( !m_addAddresseeDialog ) {
        m_addAddresseeDialog = new AddAddresseeDialog();
        connect( this, SIGNAL(foundAvailableSlots(KMobileTools::AddressbookEntry::MemorySlots)),
                 m_addAddresseeDialog, SLOT(availableSlots(KMobileTools::AddressbookEntry::MemorySlots)) );
        connect( m_addAddresseeDialog, SIGNAL(addAddressee(const KMobileTools::AddressbookEntry&)),
                 this, SLOT(requestEntryAddition(const KMobileTools::AddressbookEntry&)) );
    }

    findAvailableSlots();
    m_addAddresseeDialog->show();
}

void Addressbook::requestEntryEditing() {
    /// @todo implement me
    KMessageBox::information( m_widget, QString( "Sorry, editing not implemented yet." ) );
}

void Addressbook::requestEntryRemoval() {
    QListWidgetItem* currentItem = m_addresseeList->currentItem();

    if( currentItem ) {
        AddressbookEntryItem* entry = static_cast<AddressbookEntryItem*>( currentItem );
        if( entry ) {
            // we can only remove items with no pending actions on them
            if( entry->state() == AddressbookEntryItem::Default ) {
                int result = KMessageBox::warningYesNo(
                                            m_widget,
                                            i18n( "Do you really want to remove \"%1\" from your address book?",
                                                  entry->addressee().name() ),
                                            i18n( "Removing contact" )
                );

                if( result == KMessageBox::Yes ) {
                    entry->setState( AddressbookEntryItem::RemovalRequested );
                    m_mutex.lock();
                    m_pendingItems.append( currentItem );
                    m_mutex.unlock();

                    m_addressbook->removeAddressee( entry->addressee() );
                }
            }
        }
    }
}

void Addressbook::checkEnableActions() {
    if( m_addresseeList->currentItem() ) {
        m_editContact->setEnabled( true );
        m_deleteContact->setEnabled( true );
    }
    else {
        m_editContact->setEnabled( false );
        m_deleteContact->setEnabled( false );
    }
}

K_EXPORT_COMPONENT_FACTORY( libkmtaddressbook_service, AddressbookFactory )

AddressbookFactory::AddressbookFactory()
{
}

AddressbookFactory::~AddressbookFactory()
{
}

Addressbook *AddressbookFactory::createObject(QObject *parent, const char *classname, const QStringList& args)
{
    Q_UNUSED(classname)
    QStringList arguments( args );
    return new Addressbook( parent, arguments.at(0) );
}

#include "addressbook.moc"
