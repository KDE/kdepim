/*
    This file is part of KAddressbook.
    Copyright (c) 2000 Cornelius Schumacher <schumacher@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KAddressbook_PART_H
#define KAddressbook_PART_H
// $Id$

#include <kparts/browserextension.h>
#include <kparts/factory.h>
#include <kparts/event.h>

#include "kaddressbookiface.h"

class KInstance;
class KAboutData;
class KAddressbookBrowserExtension;

class KAddressBook;
class ActionManager;

class KAddressbookPart: public KParts::ReadOnlyPart, virtual public KAddressBookIface
{
    Q_OBJECT
  public:
    KAddressbookPart(QWidget *parentWidget, const char *widgetName,
                   QObject *parent, const char *name, const QStringList &);
    virtual ~KAddressbookPart();

    static KAboutData *createAboutData();

  public slots:
    virtual void addEmail( QString addr ) { widget->addEmail( addr ); }

    virtual ASYNC showContactEditor( QString uid ) { widget->showContactEditor( uid ); }
    virtual void newContact() { widget->newContact(); }
    virtual QString getNameByPhone( QString phone ) { return widget->getNameByPhone( phone ); }
    virtual void save() { widget->save(); }
    virtual void exit() { delete this; }
    virtual void updateEditMenu() {};

  protected:
    virtual bool openFile();
    virtual void guiActivateEvent(KParts::GUIActivateEvent *e);
    
  private:
    KAddressBook *widget;
    ActionManager *mActionManager;
    KAddressbookBrowserExtension *m_extension;
};

class KAddressbookBrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT
    friend class KAddressbookPart;
  public:
    KAddressbookBrowserExtension(KAddressbookPart *parent);
    virtual ~KAddressbookBrowserExtension();
};

#endif
