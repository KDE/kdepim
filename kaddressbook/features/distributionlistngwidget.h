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

#include <tqstringlist.h>

class TQDragEnterEvent;
class TQDragMoveEvent;
class TQDropEvent;
class TQPoint;
class TQPushButton;

namespace KABC {
    class DistributionListManager;
}

namespace KAB {
namespace DistributionListNg {

class ListBox : public KListBox
{
    Q_OBJECT
public:
    ListBox( TQWidget* parent = 0 );

signals:

    void dropped( const TQString &listName, const KABC::Addressee::List &addressees );

protected:
    //override
    void dragEnterEvent( TQDragEnterEvent *event );
    //override
    void dragMoveEvent( TQDragMoveEvent *event );
    //override
    void dropEvent( TQDropEvent *event );
};

class MainWidget : public KAB::ExtensionWidget
{
    Q_OBJECT

public:
    explicit MainWidget( KAB::Core *core, TQWidget *parent = 0, const char *name = 0 );

    //impl
    TQString title() const;

    //impl
    TQString identifier() const;


private:
    void changed( const KABC::Addressee& );

private slots:

    void deleteSelectedDistributionList();
    void editSelectedDistributionList();

    void contextMenuRequested( TQListBoxItem *item, const TQPoint &point );
    void updateEntries();
    void itemSelected( int index );
    void contactsDropped( const TQString &listName, const KABC::Addressee::List &addressees );

private:
    ListBox *mListBox;
    TQStringList mCurrentEntries;
    TQPushButton *mAddButton;
    TQPushButton *mEditButton;
    TQPushButton *mRemoveButton;
};

} // namespace DistributionListNg
} // namespace KAB

#endif // KAB_DISTRIBUTIONLISTNG_MAINWIDGET_H
