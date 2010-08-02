/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#ifndef DISTRIBUTIONLISTWIDGET_H
#define DISTRIBUTIONLISTWIDGET_H

#include <config.h> // for KDEPIM_NEW_DISTRLISTS
#include <kdialogbase.h>
#include <klistview.h>

#include "extensionwidget.h"

class TQButtonGroup;
class TQComboBox;
class TQListView;

class DistributionListView;

namespace KAB {
class Core;
}

namespace KABC {
class AddressBook;
class DistributionListManager;
}

class DistributionListWidget : public KAB::ExtensionWidget
{
    Q_OBJECT

  public:
    DistributionListWidget( KAB::Core*, TQWidget *parent, const char *name = 0 );
    virtual ~DistributionListWidget();

    void contactsSelectionChanged();

    TQString title() const;
    TQString identifier() const;

  public slots:
    void save();
    void dropped( TQDropEvent*, TQListViewItem* );

    void removeContact();

  private slots:
    void createList();
    void editList();
    void removeList();
    void addContact();
    void changeEmail();
    void updateNameCombo();
    void updateContactView();
    void selectionContactViewChanged();

  private:
#ifdef KDEPIM_NEW_DISTRLISTS
    void changed( const KABC::Addressee& dist );
#else
    void changed();
#endif
    bool alreadyExists( const TQString& distrListName ) const;

  protected:
    void dropEvent( TQDropEvent* );

  private:
    TQComboBox *mNameCombo;
    TQLabel *mEntryCountLabel;
    DistributionListView *mContactView;

#ifndef KDEPIM_NEW_DISTRLISTS
    KABC::DistributionListManager *mManager;
#endif
    TQPushButton *mCreateListButton;
    TQPushButton *mEditListButton;
    TQPushButton *mRemoveListButton;
    TQPushButton *mChangeEmailButton;
    TQPushButton *mAddContactButton;
    TQPushButton *mRemoveContactButton;
};

/**
  @short Helper class
*/
class DistributionListView : public KListView
{
  Q_OBJECT

  public:
    DistributionListView( TQWidget *parent, const char* name = 0 );

  protected:
    void dragEnterEvent( TQDragEnterEvent *e );
    void dropEvent( TQDropEvent *e );
    void viewportDragMoveEvent( TQDragMoveEvent *e );
    void viewportDropEvent( TQDropEvent *e );
};

/**
  @short Helper class
*/
class EmailSelector : public KDialogBase
{
  public:
    EmailSelector( const TQStringList &emails, const TQString &current,
                   TQWidget *parent );

    TQString selected() const;

    static TQString getEmail( const TQStringList &emails, const TQString &current,
                             TQWidget *parent, bool &canceled );

  private:
    TQButtonGroup *mButtonGroup;
    TQMap<int, TQString> mEmailMap;
};

#endif
