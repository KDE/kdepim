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

#ifndef KADDRESSBOOK_INTERFACE_H
#define KADDRESSBOOK_INTERFACE_H

#include <qstring.h>
#include <qstringlist.h>
#include <dcopobject.h>

#include "Entry.h"

class KAddressBookBackend;

namespace KAB
{
  class Command;
}

class KAddressBookInterface : public QObject, virtual public DCOPObject
{
  Q_OBJECT
  K_DCOP

  public:

    KAddressBookInterface(QString name, QString path);
    virtual ~KAddressBookInterface();

  k_dcop:

    virtual QString name();
    virtual QString path();

    // These functions start 'jobs'. The return value is an id for
    // the job. You will receive a DCOP signal when the job is complete.

    virtual int entry(QString);
    virtual int insert(KAB::Entry);
    virtual int remove(QString);
    virtual int replace(KAB::Entry);
    virtual int contains(QString);
    virtual int entryList();

    // DCOP signals emitted when jobs complete:

    // entryComplete      (int jobID, Entry)
    // insertComplete     (int jobID, bool)
    // removeComplete     (int jobID, bool)
    // replaceComplete    (int jobID, bool)
    // containsComplete   (int jobID, bool)
    // entryListComplete  (int jobID, QStringList)

  protected slots:

    void slotCommandComplete(KAB::Command *);

  private:

    static int ID_;

    QString name_;
    QString path_;

    KAddressBookBackend * backend_;

    int _queueCommand(KAB::Command *);
};

#endif
