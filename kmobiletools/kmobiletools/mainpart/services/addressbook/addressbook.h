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

#ifndef ADDRESSBOOK_H
#define ADDRESSBOOK_H

#include <KLibFactory>
#include <KAction>
#include <klistwidgetsearchline.h>
#include <KTextBrowser>

#include <libkmobiletools/coreservice.h>
#include <libkmobiletools/ifaces/guiservice.h>
#include <libkmobiletools/ifaces/actionprovider.h>

#include <libkmobiletools/addressbookentry.h>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QTimer>
#include <QtGui/QIcon>
#include <QtGui/QWidget>
#include <QtGui/QListWidget>
#include <QtCore/QMutex>

#include "addaddresseedialog.h"

namespace KMobileTools {
    class EngineXP;
    namespace Ifaces {
        class Addressbook;
    }
}
class QPoint;
/**
 * @author Matthias Lechner <matthias@lmme.de>
 *
 * @TODO implement addressee editing
 * @TODO implement html viewer
 * @TODO change KListWidgetSearchLine to a proxy model since it currently causes crashes
 */
class Addressbook : public KMobileTools::CoreService,              // base class
                    public KMobileTools::Ifaces::GuiService,       // interfaces
                    public KMobileTools::Ifaces::ActionProvider
{
    Q_OBJECT
    Q_INTERFACES(KMobileTools::Ifaces::GuiService KMobileTools::Ifaces::ActionProvider)

public:
    Addressbook( QObject* parent, const QString& deviceName );
    ~Addressbook();

    /**
     * Returns the service's name
     *
     * @return the service name
     */
    QString name() const;

    /**
     * Returns the service's icon
     *
     * @return the service icon
     */
    KIcon icon() const;

    /**
     * Returns the widget associated with the service
     *
     * @return the associated widget
     */
    QWidget* widget() const;

    /**
     * Returns a list of interfaces that an engine must have implemented
     * to use this service
     *
     * @return a list of required interfaces
     */
    QStringList requires() const;

    /**
     * Returns an action collection
     *
     * @return an action collection
     */
    QList<QAction*> actionList() const;

Q_SIGNALS:
    void foundAvailableSlots( KMobileTools::AddressbookEntry::MemorySlots );

public Q_SLOTS:
    void addEntry( const KMobileTools::AddressbookEntry& entry );
    void removeEntry( const KMobileTools::AddressbookEntry& entry );

private Q_SLOTS:
    void cleanUpItems();

    void requestEntryAddition();
    void requestEntryAddition( const KMobileTools::AddressbookEntry& entry );
    void requestEntryEditing();
    void requestEntryRemoval();

    void checkEnableActions();

    void findAvailableSlots();

    void addresseeListContextMenu( const QPoint& position );

private:
    void setupWidget();
    void setupActions();

    /// @TODO add a poll interface for this job
    QTimer* m_fetchTimer;

    KMobileTools::EngineXP* m_engine;
    KMobileTools::Ifaces::Addressbook* m_addressbook;

    QWidget* m_widget;
    QString m_deviceName;
    QList<QAction*> m_actionList;

    AddAddresseeDialog* m_addAddresseeDialog;

    QList<QListWidgetItem*> m_pendingItems;
    QMutex m_mutex;

    // gui
    KListWidgetSearchLine* m_addresseeSearch;
    QListWidget* m_addresseeList;
    KTextBrowser* m_addresseeDetails;

    // actions
    QAction* m_newContact;
    QAction* m_editContact;
    QAction* m_deleteContact;
};

class AddressbookFactory : public KLibFactory
{
   Q_OBJECT
public:
    AddressbookFactory();
    virtual ~AddressbookFactory();
    virtual Addressbook* createObject( QObject *parent, const char *classname, const QStringList &args );
};

#endif
