/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mirko Boehm <mirko@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KADDRESSBOOKDETAILEDVIEW_H
#define KADDRESSBOOKDETAILEDVIEW_H

#include <qcombobox.h>
#include <qframe.h>
#include <qlistview.h>
#include <qstring.h>
#include <qsplitter.h>
#include <qpushbutton.h>

#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kabc/field.h>
#include <klistview.h>

#include "detailsviewcontainer.h"
#include "look_details.h"
#include "../kaddressbookview.h"

class KConfig;
class KABBasicLook;

class DetailListItem : public QListViewItem
{
public:
  DetailListItem(QListView *, const KABC::Addressee&, const KABC::Field::List&, KABC::AddressBook* );

  /**
   * Sets the addressee of this item.
   */
  void setAddressee( const KABC::Addressee & );

  /**
   * Returns the addressee of this item.
   */
  KABC::Addressee addressee();

  /**
   * Refreshes the text
   */
  void refresh();

protected:
  KABC::Addressee mContact;
  KABC::Field::List mFieldList;
  KABC::AddressBook *mAddressBook;
};


/** This class incorporates the KAB MkIII detailed view into
    kaddressbook.
*/

class KAddressBookDetailedView : public KAddressBookView
{
    Q_OBJECT

public:
    KAddressBookDetailedView(KABC::AddressBook *doc,
                              QWidget *parent,
                              const char *name);
    virtual ~KAddressBookDetailedView();

    virtual QString type() const { return "Detailled"; };
    virtual QStringList selectedUids();

    virtual void readConfig(KConfig *config);
    virtual void writeConfig(KConfig *config);

    virtual void incrementalSearch(const QString &value, KABC::Field *field);

    void init(KConfig*);

signals:
    void selected(const QString &uid);

public slots:
    void refresh(QString uid = QString::null);
    void setSelected(QString uid = QString::null, bool selected = true);
    void slotAddresseeSelected(const QString& uid);
    void slotModified();
    void slotContactSelected( QListViewItem * item );
    void slotShowHideList();
    void slotShowHideDetails();

    /** Called whenever the user executes an addressee. In terms of the
    * list view, this is probably a double click
    */
    void addresseeExecuted(QListViewItem*);

private:
    void initGUI();

    KABBasicLook *m_look;
    bool showList;
    bool showDetails;

    KListView* viewTree;
    QFrame* frmDetails;
    QFrame* frmListView;
    QPushButton* pbHideDetails;
    QPushButton* pbHideList;
    QSplitter* splitter;
    ViewContainer* viewContainer;
};

#endif // KADDRESSBOOKDETAILEDVIEW_H
