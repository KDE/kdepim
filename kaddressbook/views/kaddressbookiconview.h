/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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

#ifndef KADDRESSBOOKICONVIEW_H
#define KADDRESSBOOKICONVIEW_H

#include <QString>
//Added by qt3to4:
#include <QList>
#include <QDropEvent>
#include <k3iconview.h>
#include "kaddressbookview.h"

class Q3IconViewItem;
class KConfigGroup;
class AddresseeIconView;
class AddresseeIconViewItem;

namespace KABC { class AddressBook; }

/** This is an example kaddressbook view that is implemented using
* K3IconView. This view is not the most useful view, but it displays
* how simple implementing a new view can be.
*/
class KAddressBookIconView : public KAddressBookView
{
  Q_OBJECT

  public:
    KAddressBookIconView( KAB::Core *core, QWidget *parent );
    virtual ~KAddressBookIconView();

    virtual QStringList selectedUids();
    virtual QString type() const { return "Icon"; }
    virtual KABC::Field *sortField() const;
    virtual void readConfig( KConfigGroup &config );

    void scrollUp();
    void scrollDown();

  public slots:
    void refresh( const QString &uid = QString() );
    void setSelected( const QString &uid = QString(), bool selected = true );
    virtual void setFirstSelected( bool selected = true );

  protected slots:
    void addresseeExecuted( Q3IconViewItem *item );
    void addresseeSelected();
    void rmbClicked( Q3IconViewItem*, const QPoint& );

  private:
    AddresseeIconView *mIconView;
    QList<AddresseeIconViewItem*> mIconList;
};


class AddresseeIconView : public K3IconView
{
  Q_OBJECT

  public:
    AddresseeIconView( QWidget *parent, const char *name = 0 );
    ~AddresseeIconView();

  signals:
    void addresseeDropped( QDropEvent* );
    void startAddresseeDrag();

  protected:
    virtual Q3DragObject *dragObject();

  protected slots:
    void itemDropped( QDropEvent*, const QList<Q3IconDragItem>& );
};
#endif
