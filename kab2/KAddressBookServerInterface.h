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

#ifndef KADDRESSBOOK_SERVER_INTERFACE_H
#define KADDRESSBOOK_SERVER_INTERFACE_H

#include <qstring.h>
#include <qstringlist.h>
#include <dcopobject.h>

#include <kab2/FormatDefinition.h>

class KAddressBookInterface;

class KAddressBookServerInterface : virtual public DCOPObject
{
  K_DCOP

  public:

    KAddressBookServerInterface();
    virtual ~KAddressBookServerInterface();

k_dcop:

    virtual QStringList formatDefinitionList();
    virtual KAB::FormatDefinition formatDefinition(QString);
    virtual KAB::FormatDefinition defaultFormatDefinition();

    virtual bool addFormatDefinition(KAB::FormatDefinition);
    virtual bool updateFormatDefinition(KAB::FormatDefinition);

    virtual bool removeFormatDefinition(QString);
    virtual bool hasFormatDefinition(QString);

    virtual QStringList list();
    virtual bool remove(QString);
    virtual bool create(QString name, QString location, QString formatName);

  private:

    void _updateFormatDefinitionList();
    void _readConfig();
    void _writeConfig();

    QList<KAddressBookInterface> addressBookList_;
    QStringList formatDefinitionList_;
};

#endif

