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

#ifndef ADDADDRESSEEDIALOG_H
#define ADDADDRESSEEDIALOG_H

#include <QtGui/QWidget>
#include <KDialog>

#include <libkmobiletools/addressbookentry.h>

class KPushButton;
class QComboBox;
class QTableWidget;
class KLineEdit;
/**
 * @author Matthias Lechner <matthias@lmme.de>
 */
class AddAddresseeDialog : public KDialog
{
    Q_OBJECT
public:
    AddAddresseeDialog( QWidget* parent = 0 );

    ~AddAddresseeDialog();

public Q_SLOTS:
    void availableSlots( KMobileTools::AddressbookEntry::MemorySlots );
    void accept();
    void show();

private Q_SLOTS:
    void addPhoneNumber();
    void removePhoneNumber();

Q_SIGNALS:
    void addAddressee( const KMobileTools::AddressbookEntry& );

private:
    void setupGui();

    QString memorySlotToString( KMobileTools::AddressbookEntry::MemorySlot memorySlot );

    QWidget* m_widget;
    KLineEdit* m_name;
    KLineEdit* m_email;

    KLineEdit* m_phoneNumber;
    QTableWidget* m_phoneNumberTable;
    QComboBox* m_phoneNumberTypes;
    KPushButton* m_addPhoneNumber;
    KPushButton* m_removePhoneNumber;

    QComboBox* m_storageLocation;
};

#endif
