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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef LDAPSEARCHDIALOG_H
#define LDAPSEARCHDIALOG_H

#include <qptrlist.h>

#include <kabc/addressbook.h>
#include <ldapclient.h>
#include <kdialogbase.h>

class KAddressBookTableView;
class KComboBox;
class KLineEdit;

class QCheckBox;
class QListView;
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
    virtual void slotUser2();
    virtual void slotUser3();

  protected:
    QString selectedEMails() const;

    virtual void closeEvent( QCloseEvent* );

  private:
    void saveSettings();

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
};

#endif
