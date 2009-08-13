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

#include <QtGui/QCloseEvent>

#include <kabc/addressee.h>
#include <kdialog.h>
#include <ldapclient.h>

class KComboBox;
class KLineEdit;

class QCheckBox;
class QPushButton;
class QTableView;
class ContactListModel;

class LDAPSearchDialog : public KDialog
{
  Q_OBJECT

  public:
    LDAPSearchDialog( QWidget* parent = 0 );
    ~LDAPSearchDialog();

    KABC::Addressee::List selectedContacts() const;

  protected Q_SLOTS:
    virtual void slotHelp();
    virtual void slotUser1();
    virtual void slotUser2();

  protected:
    virtual void closeEvent( QCloseEvent* );

  private Q_SLOTS:
    void slotAddResult( const KPIM::LdapClient &client, const KLDAP::LdapObject& obj );
    void slotSetScope( bool rec );
    void slotStartSearch();
    void slotStopSearch();
    void slotSearchDone();
    void slotError( const QString& );
    void slotSelectAll();
    void slotUnselectAll();

  private:
    void saveSettings();
    void restoreSettings();
    void cancelQuery();

    QString makeFilter( const QString& query, const QString& attr, bool startsWith ) const;

    static KABC::Addressee convertLdapAttributesToAddressee( const KLDAP::LdapAttrMap& attrs );

#if 0 //sbesauer
    KABC::DistributionList *selectDistributionList();
#endif

    int mNumHosts;
    QList<KPIM::LdapClient*> mLdapClientList;
    bool mIsConfigured;
    KABC::Addressee::List mSelectedContacts;

    KComboBox* mFilterCombo;
    KComboBox* mSearchType;
    KLineEdit* mSearchEdit;

    QCheckBox* mRecursiveCheckbox;
    QTableView* mResultView;
    QPushButton* mSearchButton;
    ContactListModel* mModel;

    class Private;
    Private* const d;
};

#endif
