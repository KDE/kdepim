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

#ifndef KPIM_DISTRIBUTIONLISTEDITOR_H
#define KPIM_DISTRIBUTIONLISTEDITOR_H

#include <KDialog>

namespace KABC { class AddressBook; }

namespace KPIM {

class DistributionList;

namespace DistributionListEditor {

class EditorWidgetPrivate;
class EditorWidget : public KDialog
{
    Q_OBJECT
public:
    explicit EditorWidget( KABC::AddressBook* book, QWidget* parent = 0 );
    ~EditorWidget();

    void setDistributionList( const KPIM::DistributionList& list );
    KPIM::DistributionList distributionList() const;

    void saveList();

private Q_SLOTS:
    //override
    void slotButtonClicked( int button );
    void lineTextChanged( int id );

private:
    EditorWidgetPrivate* const d;
};

} // namespace DisributionListEditor
} // namespace KPIM

#endif // KPIM_DISTRIBUTIONLISTEDITOR_H
