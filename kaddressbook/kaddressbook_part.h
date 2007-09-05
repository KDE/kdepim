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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KADDRESSBOOK_PART_H
#define KADDRESSBOOK_PART_H

#include <kparts/event.h>
#include <kparts/factory.h>
#include <kparts/part.h>

#include "kaddressbookiface.h"

class KAboutData;
class KInstance;

class KABCore;

class KAddressbookPart: public KParts::ReadOnlyPart, virtual public KAddressBookIface
{
  Q_OBJECT

  public:
    KAddressbookPart( QWidget *parentWidget, const char *widgetName,
                      QObject *parent, const char *name, const QStringList& );
    virtual ~KAddressbookPart();

    static KAboutData *createAboutData();

    virtual void saveToProfile( const QString& path ) const;
    virtual void loadProfile( const QString& path );
  public slots:
    virtual void addEmail( QString addr );
    virtual void importVCard( const KURL& url );
    virtual void importVCardFromData( const QString& vCard );
    virtual ASYNC showContactEditor( QString uid );
    virtual void newContact();
    virtual void newDistributionList();
    virtual QString getNameByPhone( QString phone );
    virtual void save();
    virtual void exit();
    virtual bool openURL( const KURL &url );
    virtual bool handleCommandLine();

  protected:
    virtual bool openFile();
    virtual void guiActivateEvent( KParts::GUIActivateEvent* );

  private:
    KABCore *mCore;
};

#endif
