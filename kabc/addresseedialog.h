/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KABC_ADDRESSEEDIALOG_H
#define KABC_ADDRESSEEDIALOG_H
// $Id$

#include <qdict.h>

#include <kdialogbase.h>
#include <klistview.h>
#include <klineedit.h>

#include "addressbook.h"

namespace KABC {

class AddresseeItem : public QListViewItem
{
  public:
    AddresseeItem( QListView *parent, const Addressee &addressee ) :
      QListViewItem( parent ),
      mAddressee( addressee )
    {
      setText( 0, addressee.realName() );
      setText( 1, addressee.preferredEmail() );
    }

    Addressee addressee() const
    {
      return mAddressee;
    }

  private:
    Addressee mAddressee;
};

/**
  @short Dialog for selecting address book entries.

  This class provides a dialog for selecting entries from the standard KDE
  address book. Use the @ref getAddressee() function to open a modal dialog,
  returning an address book entry.

  In the dialog you can select an entry from the list with the mouse or type in
  the first letters of the name or email address you are searching for. The
  entry matching best is automatically selected. Use double click, pressing
  return or pressing the ok button to return the selected addressee to the
  application.
*/
class AddresseeDialog : public KDialogBase {
    Q_OBJECT
  public:
    /**
      Construct addressbook entry select dialog.

      @param parent parent widget
    */
    AddresseeDialog( QWidget *parent );
    virtual ~AddresseeDialog();

    /**
      Open addressee select dialog and return the entry selected by the user.
      If the user doesn't selectan entry or presses cancel, the returned
      addressee is empty.
    */
    static Addressee getAddressee( QWidget *parent );

  private slots:
    void selectItem( const QString & );
    void updateEdit( QListViewItem *item );

  private:
    void loadAddressBook();
    void addCompletionItem( const QString &str, QListViewItem *item );

    KListView *mAddresseeList;
    KLineEdit *mAddresseeEdit;

    AddressBook *mAddressBook;

    QDict<QListViewItem> mItemDict;
};

}

#endif
