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

#ifndef KAB_DISTRIBUTIONLISTNG_WIDGET_H
#define KAB_DISTRIBUTIONLISTNG_WIDGET_H

#include <QtCore/QStringList>
#include <QtGui/QListWidget>

#include <kabc/addressee.h>

#include "extensionwidget.h"

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QPoint;
class QToolButton;

namespace KABC {
    class DistributionList;
}

namespace KAB {
namespace DistributionListNg {

class ListBox : public QListWidget
{
    Q_OBJECT
public:
    ListBox( QWidget* parent = 0 );

Q_SIGNALS:

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
    explicit MainWidget( KAB::Core *core, QWidget *parent = 0 );

    //impl
    QString title() const;

    //impl
    QString identifier() const;

private Q_SLOTS:
    void deleteSelectedDistributionList();
    void editSelectedDistributionList();

    void contextMenuRequested( const QPoint &point );
    void updateEntries();
    void itemSelected( int index );
    void contactsDropped( const QString &listName, const KABC::Addressee::List &addressees );

private:
    void changed( const KABC::DistributionList* );

    ListBox *mListBox;
    QStringList mCurrentEntries;
    QToolButton *mAddButton;
    QToolButton *mEditButton;
    QToolButton *mRemoveButton;
};

} // namespace DistributionListNg
} // namespace KAB

#endif // KAB_DISTRIBUTIONLISTNG_MAINWIDGET_H
