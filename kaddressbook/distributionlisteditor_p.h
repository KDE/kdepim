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

#include <QtCore/QString>

#include <libkdepim/addresseelineedit.h>
#include <libkdepim/distributionlist.h>

class QToolButton;

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
    explicit LineEdit( QWidget* parent = 0 );

  protected:
    void addContact( const KABC::Addressee &addr, int weight, int source ); // reimpl
};


class Line : public QWidget
{
    Q_OBJECT
public:
    explicit Line( KABC::AddressBook* book, QWidget* parent = 0 );

    void setEntry( const KPIM::DistributionList::Entry& entry );
    KPIM::DistributionList::Entry entry() const;
    void setFocusToLineEdit();

Q_SIGNALS:
    void cleared();
    void textChanged();

private:
    KABC::Addressee findAddressee( const QString& name, const QString& email ) const;

private Q_SLOTS:
    void textChanged( const QString& );

private:
    QString m_uid;
    QString m_initialText;
    LineEdit* m_lineEdit;
    KABC::AddressBook* m_addressBook;
};

} // namespace DisributionListEditor
} // namespace KPIM

#endif // KPIM_DISTRIBUTIONLISTEDITOR_P_H


