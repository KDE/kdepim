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

#ifndef LDAPSEARCHDIALOGIMPL_H
#define LDAPSEARCHDIALOGIMPL_H

#include <qptrlist.h>

#include <kabc/addressbook.h>

#include "ldapsearchdialog.h"
#include "kldapclient.h"

class KAddressBookTableView;

class LDAPSearchDialogImpl : public LDAPSearchDialog
{ 
  Q_OBJECT

public:
  LDAPSearchDialogImpl( KABC::AddressBook *ab, QWidget* parent, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
  ~LDAPSearchDialogImpl();
  bool isOK() const { return bOK; }

  void rereadConfig();

signals:
  void addresseesAdded();

protected slots:
  void slotAddResult( const KLdapObject& obj );
  void slotSetScope( bool rec );
  void slotStartSearch();
  void slotStopSearch();
  void slotSearchDone();
  void slotAddSelectedContacts();
  void slotSendMail();
  void slotError( const QString& );

protected:
  QString selectedEMails() const;

  virtual void closeEvent( QCloseEvent* );

private:
  QString makeFilter( const QString& query, const QString& attr );

  void cancelQuery();

  int numHosts;
  QPtrList<KLdapClient> ldapclientlist;
  bool bOK;
  KABC::AddressBook *mAddressBook;
};

#endif // LDAPSEARCHDIALOGIMPL_H
