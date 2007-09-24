/* ldapsearchdialogimpl.h - LDAP access
 *      Copyright (C) 2002 Klar�lvdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef LDAPSEARCHDIALOG_H
#define LDAPSEARCHDIALOG_H

#include "config.h"

#include <qptrlist.h>

#include <kabc/addressbook.h>
#include <ldapclient.h>
#include <kdialogbase.h>

#ifdef KDEPIM_NEW_DISTRLISTS
#include <libkdepim/distributionlist.h>
#endif

class KAddressBookTableView;
class KComboBox;
class KLineEdit;

class QCheckBox;
class QListView;
class QPushButton;
class KABCore;
class ContactListItem;

namespace KABC {
    class Resource;
}

class LDAPSearchDialog : public KDialogBase
{
  Q_OBJECT

  public:
    LDAPSearchDialog( KABC::AddressBook *ab, KABCore *core, QWidget* parent, const char* name = 0 );
    ~LDAPSearchDialog();

    bool isOK() const { return mIsOK; }

    void restoreSettings();

  signals:
    void addresseesAdded();

  protected slots:
    void slotAddResult( const KPIM::LdapObject& obj );
    void slotSetScope( bool rec );
    void slotStartSearch();
    void slotStopSearch();
    void slotSearchDone();
    void slotError( const QString& );
    virtual void slotHelp();
    virtual void slotUser1();
    virtual void slotUser2();
    void slotSelectAll();
    void slotUnselectAll();
    /**
     * Traverses the given items and adds them to the given resource,
     * unless they already exist. Returns the list of both the added
     * and the existing contacts.
     */
    KABC::Addressee::List importContactsUnlessTheyExist( const QValueList<ContactListItem*>& items, KABC::Resource * const resource );

  protected:
    QString selectedEMails() const;

    virtual void closeEvent( QCloseEvent* );

  private:
    void saveSettings();
    static KABC::Addressee convertLdapAttributesToAddressee( const KPIM::LdapAttrMap& attrs );
#ifdef KDEPIM_NEW_DISTRLISTS
    KPIM::DistributionList selectDistributionList();
#endif

    QString makeFilter( const QString& query, const QString& attr, bool startsWith );

    void cancelQuery();

    int mNumHosts;
    QPtrList<KPIM::LdapClient> mLdapClientList;
    bool mIsOK;
    KABC::AddressBook *mAddressBook;
    KABCore *mCore;

    KComboBox* mFilterCombo;
    KComboBox* mSearchType;
    KLineEdit* mSearchEdit;

    QCheckBox* mRecursiveCheckbox;
    QListView* mResultListView;
    QPushButton* mSearchButton;
    class Private;
    Private* const d;
};

#endif
