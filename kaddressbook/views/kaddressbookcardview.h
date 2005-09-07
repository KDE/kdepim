#ifndef KADDRESSBOOKCARDVIEW_H
#define KADDRESSBOOKCARDVIEW_H

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

#include <qstring.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDropEvent>
#include <kiconview.h>

#include "cardview.h"
#include "kaddressbookview.h"

class QDragEntryEvent;
class QDropEvent;
class KConfig;
class AddresseeCardView;

/**
  This view uses the CardView class to create a card view. At some
  point in the future I think this will be the default view of
  KAddressBook.
 */
class KAddressBookCardView : public KAddressBookView
{
  Q_OBJECT

  public:
    KAddressBookCardView( KAB::Core *core, QWidget *parent,
                          const char *name = 0 );
    virtual ~KAddressBookCardView();

    virtual QStringList selectedUids();
    virtual QString type() const { return "Card"; }
    virtual KABC::Field *sortField() const;

    virtual void readConfig( KConfig *config );
    virtual void writeConfig( KConfig *config );

    void scrollUp();
    void scrollDown();

  public slots:
    void refresh( const QString &uid = QString() );
    void setSelected( const QString &uid = QString(), bool selected = true );
    virtual void setFirstSelected( bool selected = true );

  protected slots:
    void addresseeExecuted( CardViewItem* );
    void addresseeSelected();
    void rmbClicked( CardViewItem*, const QPoint& );

  private:
    AddresseeCardView *mCardView;
    bool mShowEmptyFields;
};

class AddresseeCardView : public CardView
{
  Q_OBJECT
  public:
    AddresseeCardView( QWidget *parent, const char *name = 0 );
    ~AddresseeCardView();

  signals:
    void startAddresseeDrag();
    void addresseeDropped( QDropEvent* );

  protected:
    virtual void dragEnterEvent( QDragEnterEvent* );
    virtual void dropEvent( QDropEvent* );
    virtual void startDrag();
};

#endif
