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

#include <tqstring.h>
#include <kiconview.h>
#include "kaddressbookview.h"

class TQIconViewItem;
class KConfig;
class AddresseeIconView;
class AddresseeIconViewItem;

namespace KABC { class AddressBook; }

/** This is an example kaddressbook view that is implemented using
* KIconView. This view is not the most useful view, but it displays
* how simple implementing a new view can be.
*/
class KAddressBookIconView : public KAddressBookView
{
  Q_OBJECT

  public:
    KAddressBookIconView( KAB::Core *core, TQWidget *parent,
                          const char *name = 0 );
    virtual ~KAddressBookIconView();

    virtual TQStringList selectedUids();
    virtual TQString type() const { return "Icon"; }
    virtual KABC::Field *sortField() const;
    virtual void readConfig( KConfig *config );

    void scrollUp();
    void scrollDown();

  public slots:
    void refresh( const TQString &uid = TQString() );
    void setSelected( const TQString &uid = TQString(), bool selected = true );
    virtual void setFirstSelected( bool selected = true );

  protected slots:
    void addresseeExecuted( TQIconViewItem *item );
    void addresseeSelected();
    void rmbClicked( TQIconViewItem*, const TQPoint& );

  private:
    AddresseeIconView *mIconView;
    TQPtrList<AddresseeIconViewItem> mIconList;
};


class AddresseeIconView : public KIconView
{
  Q_OBJECT

  public:
    AddresseeIconView( TQWidget *parent, const char *name = 0 );
    ~AddresseeIconView();

  signals:
    void addresseeDropped( TQDropEvent* );
    void startAddresseeDrag();

  protected:
    virtual TQDragObject *dragObject();

  protected slots:
    void itemDropped( TQDropEvent*, const TQValueList<TQIconDragItem>& );
};
#endif
