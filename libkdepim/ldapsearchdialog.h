/* ldapsearchdialogimpl.h - LDAP access
 *      Copyright (C) 2002 Klar�vdalens Datakonsult AB
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

#include <tqptrlist.h>

#include <ldapclient.h>
#include <kdialogbase.h>
#include <klineedit.h>

class KComboBox;

class TQCheckBox;
class TQListView;
class TQPushButton;

namespace KPIM {

class LDAPSearchDialog : public KDialogBase
{ 
  Q_OBJECT

  public:
    LDAPSearchDialog( TQWidget* parent, const char* name = 0 );
    ~LDAPSearchDialog();

    bool isOK() const { return mIsOK; }

    void restoreSettings();

    void setSearchText( const TQString &text ) { mSearchEdit->setText( text ); }
    TQString selectedEMails() const;
  signals:
    void addresseesAdded();

  protected slots:
    void slotAddResult( const KPIM::LdapObject& obj );
    void slotSetScope( bool rec );
    void slotStartSearch();
    void slotStopSearch();
    void slotSearchDone();
    void slotError( const TQString& );
    virtual void slotHelp();
    virtual void slotUser1();
    virtual void slotUser2();
    virtual void slotUser3();

  protected:

    virtual void closeEvent( TQCloseEvent* );

  private:
    void saveSettings();

    TQString makeFilter( const TQString& query, const TQString& attr, bool startsWith );

    void cancelQuery();

    int mNumHosts;
    TQPtrList<KPIM::LdapClient> mLdapClientList;
    bool mIsOK;
    KComboBox* mFilterCombo;
    KComboBox* mSearchType;
    KLineEdit* mSearchEdit;

    TQCheckBox* mRecursiveCheckbox;
    TQListView* mResultListView;
    TQPushButton* mSearchButton;
};


}
#endif
