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

#ifndef KADDRESSBOOK_BACKEND_H
#define KADDRESSBOOK_BACKEND_H

#include <qobject.h>
#include <qqueue.h>

#include <qstring.h>
#include <qstringlist.h>

namespace KAB
{
  class Command;
}

class KAddressBookBackend : public QObject
{
  Q_OBJECT

  public:

    KAddressBookBackend
      (
       QString id,
       QString path,
       QObject * parent = 0,
       const char * name = 0
      );

    virtual ~KAddressBookBackend();

    bool initSuccess() const;

    QString id() const;
    QString path() const;

    virtual void runCommand(KAB::Command *) = 0;
    void queueCommand(KAB::Command *);

  protected:

    void setInitSuccess();

  protected slots:

    void slotCommandComplete(KAB::Command *);

  signals:

    void commandComplete(KAB::Command *);

  private:

    void _runQueue();

    QString id_;
    QString path_;

    bool initSuccess_;
    bool locked_;

    QQueue<KAB::Command> commandQueue_;
};

#endif
