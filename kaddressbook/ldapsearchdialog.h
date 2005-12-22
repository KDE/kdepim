/* ldapsearchdialogimpl.h - LDAP access
 *      Copyright (C) 2002 Klarälvdalens Datakonsult AB
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

#include <q3ptrlist.h>
//Added by qt3to4:
#include <QCloseEvent>

#include <kabc/addressbook.h>
#include <ldapclient.h>
#include <kdialogbase.h>

class KAddressBookTableView;
class KComboBox;
class KLineEdit;

class QCheckBox;
class Q3ListView;
class QPushButton;
class KABCore;

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
    void slotSelectAll();
    void slotUnselectAll();

  protected:
    QString selectedEMails() const;

    virtual void closeEvent( QCloseEvent* );

  private:
    void saveSettings();

    QString makeFilter( const QString& query, const QString& attr, bool startsWith );

    void cancelQuery();

    int mNumHosts;
    QList<KPIM::LdapClient*> mLdapClientList;
    bool mIsOK;
    KABC::AddressBook *mAddressBook;
    KABCore *mCore;

    KComboBox* mFilterCombo;
    KComboBox* mSearchType;
    KLineEdit* mSearchEdit;

    QCheckBox* mRecursiveCheckbox;
    Q3ListView* mResultListView;
    QPushButton* mSearchButton;
};

#endif
