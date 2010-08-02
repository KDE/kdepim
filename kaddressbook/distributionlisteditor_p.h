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

#ifndef KPIM_DISTRIBUTIONLISTEDITOR_P_H
#define KPIM_DISTRIBUTIONLISTEDITOR_P_H

#include <libkdepim/addresseelineedit.h>
#include <libkdepim/distributionlist.h>

#include <tqpushbutton.h>
#include <tqstring.h>

class TQToolButton;

namespace KABC {
    class Addressee;
    class AddressBook;
}

namespace KPIM {
namespace DistributionListEditor {

class LineEdit : public KPIM::AddresseeLineEdit
{
    Q_OBJECT
public:
    explicit LineEdit( TQWidget* parent = 0 );
};


class Line : public QWidget
{
    Q_OBJECT
public:
    explicit Line( KABC::AddressBook* book, TQWidget* parent = 0 );

    void setEntry( const KPIM::DistributionList::Entry& entry );
    KPIM::DistributionList::Entry entry() const; 
    void setFocusToLineEdit();
    
signals:
    void cleared();
    void textChanged();

private:
    KABC::Addressee findAddressee( const TQString& name, const TQString& email ) const; 

private slots:
    void textChanged( const TQString& );

private:
    TQString m_uid;
    TQString m_initialText;
    LineEdit* m_lineEdit;
    TQToolButton* m_clearButton;
    KABC::AddressBook* m_addressBook;
};

} // namespace DisributionListEditor
} // namespace KPIM

#endif // KPIM_DISTRIBUTIONLISTEDITOR_P_H


