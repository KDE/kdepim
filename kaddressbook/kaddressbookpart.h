/*
  This file is part of KAddressBook.

  Copyright (c) 2009 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KADDRESSBOOKPART_H
#define KADDRESSBOOKPART_H

#include <KParts/Event>
#include <KParts/Part>
#include <kparts/readonlypart.h>

class MainWidget;

class KAddressBookPart: public KParts::ReadOnlyPart
{
  Q_OBJECT

  public:
    KAddressBookPart( QWidget *parentWidget, QObject *parent, const QVariantList & );
    virtual ~KAddressBookPart();

public Q_SLOTS:
    void newContact();
    void newGroup();
    void updateQuickSearchText();

  protected:
    virtual bool openFile();
    virtual void guiActivateEvent( KParts::GUIActivateEvent * );
    void initAction();
  private:
    MainWidget *mMainWidget;
};

#endif
