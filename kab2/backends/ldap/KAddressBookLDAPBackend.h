/*
    KAddressBook version 2
    
    Copyright (C) 1999 The KDE PIM Team <kde-pim@kde.org>
    
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
*/

#ifndef KADDRESSBOOK_FILE_BACKEND_H
#define KADDRESSBOOK_FILE_BACKEND_H

// LDAP includes
#include <lber.h>
#include <ldap.h>

// KDE includes
#include <klibloader.h>

#include "KAddressBookBackend.h"

class QTimer;

class KAddressBookLDAPBackendFactory : public KLibFactory
{
  Q_OBJECT

  public:

    KAddressBookLDAPBackendFactory(QObject * parent = 0, const char * name = 0);
    ~KAddressBookLDAPBackendFactory();

  protected:

    QObject * createObject
      (
       QObject * parent = 0,
       const char * name = 0,
       const char * /* classname */ = "QObject",
       const QStringList & args = QStringList()
      );
};

class KAddressBookLDAPBackend : public KAddressBookBackend
{
  Q_OBJECT

  public:

    KAddressBookLDAPBackend
      (
       QString name,
       QString path,
       QObject * parent = 0,
       const char * name = 0
      );

    virtual ~KAddressBookLDAPBackend();

    virtual void runCommand(KAB::Command * c);

  protected slots:

    void slotPoll();

  private:

    QTimer * pollTimer_;

    int expectedMessage_;

    LDAP * client_;

    KAB::Command * currentCommand_;
};

#endif
