/*
    This file is part of KAddressBook.
    Copyright (c) 2007 Klaralvdalens Datakonsult AB <frank@kdab.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KPIM_DISTRIBUTIONLISTPICKER_H
#define KPIM_DISTRIBUTIONLISTPICKER_H

#include <kdialogbase.h>

#include <qstring.h>

class KListBox;

namespace KABC {
    class AddressBook;
}

namespace KPIM {

class DistributionListPickerDialog : public KDialogBase
{
    Q_OBJECT
public:
    explicit DistributionListPickerDialog( KABC::AddressBook* book, QWidget* parent = 0 );
    QString selectedDistributionList() const;

private slots:

    //override
    void slotOk();

    //override
    void slotCancel();

    //override
    void slotUser1();

    void entrySelected( const QString& name );

private:
    KABC::AddressBook* m_book;
    KListBox* m_listBox;
    QString m_selectedDistributionList;
};
    
} //namespace KPIM

#endif // KPIM_DISTRIBUTIONLISTPICKER_H 
