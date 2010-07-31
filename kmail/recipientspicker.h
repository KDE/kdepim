/*
    This file is part of KMail.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef RECIPIENTSPICKER_H
#define RECIPIENTSPICKER_H

#include <config.h> // for KDEPIM_NEW_DISTRLISTS

#include "recipientseditor.h"

#include <klistview.h>
#include <klistviewsearchline.h>
#include <kabc/addressee.h>
#include <kabc/stdaddressbook.h>

#include <tqwidget.h>
#include <tqdialog.h>
#include <tqtooltip.h>

class QComboBox;

#ifdef KDEPIM_NEW_DISTRLISTS
#include <libkdepim/distributionlist.h>
#else
namespace KABC {
class DistributionList;
class DistributionListManager;
}
#endif

namespace KPIM {
class  LDAPSearchDialog;
}

class RecipientItem
{
  public:
    typedef TQValueList<RecipientItem *> List;

#ifdef KDEPIM_NEW_DISTRLISTS
    RecipientItem( KABC::AddressBook *ab );
    void setDistributionList( KPIM::DistributionList& );
    KPIM::DistributionList& distributionList();
#else
    RecipientItem();
    void setDistributionList( KABC::DistributionList * );
    KABC::DistributionList * distributionList();
#endif
    void setAddressee( const KABC::Addressee &, const TQString &email );

    void setRecipientType( const TQString &type );
    TQString recipientType() const;

    TQString recipient() const;

    TQPixmap icon() const;
    TQString name() const;
    TQString email() const;

    TQString key() const { return mKey; }

    TQString tooltip() const;

  private:
#ifdef KDEPIM_NEW_DISTRLISTS
    TQString createTooltip( KPIM::DistributionList & ) const;
#else
    TQString createTooltip( KABC::DistributionList * ) const;
#endif

    KABC::Addressee mAddressee;
    TQString mName;
    TQString mEmail;
    TQString mRecipient;
#ifdef KDEPIM_NEW_DISTRLISTS
    KPIM::DistributionList mDistributionList;
    KABC::AddressBook *mAddressBook;
#else
    KABC::DistributionList *mDistributionList;
#endif
    TQString mType;
    TQString mTooltip;
    
    TQPixmap mIcon;

    TQString mKey;
};

class RecipientViewItem : public KListViewItem
{
  public:
    RecipientViewItem( RecipientItem *, KListView * );

    RecipientItem *recipientItem() const;

  private:
    RecipientItem *mRecipientItem;
};

class RecipientsListToolTip : public QToolTip
{
  public:
    RecipientsListToolTip( TQWidget *parent, KListView * );

  protected:
    void maybeTip( const TQPoint &pos );

  private:
    KListView *mListView;
};

class RecipientsCollection
{
  public:
    RecipientsCollection( const TQString & );
    ~RecipientsCollection();

    void setReferenceContainer( bool );
    bool isReferenceContainer() const;

    void setTitle( const TQString & );
    TQString title() const;

    void addItem( RecipientItem * );

    RecipientItem::List items() const;

    bool hasEquivalentItem( RecipientItem * ) const;
    RecipientItem * getEquivalentItem( RecipientItem *) const;

    void clear();

    void deleteAll();

    TQString id() const;

  private:
    // flag to indicate if this collection contains just references
    // or should manage memory (de)allocation as well.
    bool mIsReferenceContainer;
    TQString mId;
    TQString mTitle;
    TQMap<TQString, RecipientItem *> mKeyMap;
};

class SearchLine : public KListViewSearchLine
{
    Q_OBJECT
  public:
    SearchLine( TQWidget *parent, KListView *listView );

  signals:
    void downPressed();

  protected:
    void keyPressEvent( TQKeyEvent * );
};

using namespace KABC;

class RecipientsPicker : public QDialog
{
    Q_OBJECT
  public:
    RecipientsPicker( TQWidget *parent );
    ~RecipientsPicker();

    void setRecipients( const Recipient::List & );
    void updateRecipient( const Recipient & );

    void setDefaultType( Recipient::Type );

  signals:
    void pickedRecipient( const Recipient & );

  protected:
    void initCollections();
    void insertDistributionLists();
    void insertRecentAddresses();
    void insertCollection( RecipientsCollection *coll );

    void keyPressEvent( TQKeyEvent *ev );

    void readConfig();
    void writeConfig();

    void pick( Recipient::Type );

    void setDefaultButton( TQPushButton *button );

    void rebuildAllRecipientsList();

  protected slots:
    void updateList();
    void slotToClicked();
    void slotCcClicked();
    void slotBccClicked();
    void slotPicked( TQListViewItem * );
    void slotPicked();
    void setFocusList();
    void resetSearch();
    void insertAddressBook( AddressBook * );
    void slotSearchLDAP();
    void ldapSearchResult();
  private:
    KABC::StdAddressBook *mAddressBook;

    TQComboBox *mCollectionCombo;
    KListView *mRecipientList;
    KListViewSearchLine *mSearchLine;

    TQPushButton *mToButton;
    TQPushButton *mCcButton;
    TQPushButton *mBccButton;

    TQPushButton *mSearchLDAPButton;
    KPIM::LDAPSearchDialog *mLdapSearchDialog;

    TQMap<int,RecipientsCollection *> mCollectionMap;
    RecipientsCollection *mAllRecipients;
    RecipientsCollection *mDistributionLists;
    RecipientsCollection *mSelectedRecipients;

#ifndef KDEPIM_NEW_DISTRLISTS
    KABC::DistributionListManager *mDistributionListManager;
#endif

    Recipient::Type mDefaultType;
};

#endif
