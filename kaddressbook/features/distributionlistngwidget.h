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

#ifndef KAB_DISTRIBUTIONLISTNG_MAINWIDGET_H
#define KAB_DISTRIBUTIONLISTNG_MAINWIDGET_H

#include "extensionwidget.h"

#include <kabc/addressee.h>

#include <klistbox.h>

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QPoint;

namespace KABC {
    class DistributionListManager;
}

namespace KAB {
namespace DistributionListNg {

class ListBox : public KListBox
{
    Q_OBJECT
public:
    ListBox( QWidget* parent = 0 );

signals:
    
    void dropped( const QString &listName, const KABC::Addressee::List &addressees ); 

protected:
    //override
    void dragEnterEvent( QDragEnterEvent *event );
    //override
    void dragMoveEvent( QDragMoveEvent *event ); 
    //override
    void dropEvent( QDropEvent *event );
};

class MainWidget : public KAB::ExtensionWidget
{
    Q_OBJECT

public:
    explicit MainWidget( KAB::Core *core, QWidget *parent = 0, const char *name = 0 );

    //impl
    QString title() const;

    //impl
    QString identifier() const;

private slots:
    
    void contextMenuRequested( QListBoxItem *item, const QPoint &point );
    void updateEntries();
    
    void contactsDropped( const QString &listName, const KABC::Addressee::List &addressees ); 

private:
    ListBox *mListBox;
    KABC::DistributionListManager *mManager;
};

} // namespace DistributionListNg
} // namespace KAB

#endif // KAB_DISTRIBUTIONLISTNG_MAINWIDGET_H
