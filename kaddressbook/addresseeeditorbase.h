/*
    This file is part of KAddressBook.
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef ADDRESSEEEDITORBASE_H
#define ADDRESSEEEDITORBASE_H

#include <kabc/addressee.h>

#include "extensionwidget.h"

namespace KAB {
class Core;
}

class AddresseeEditorBase : public KAB::ExtensionWidget
{
  public:
    AddresseeEditorBase( KAB::Core *core, bool isExtension,
                         QWidget *parent, const char *name = 0 )
      : KAB::ExtensionWidget( core, parent, name ), mIsExtension( isExtension )
    {
    }

    ~AddresseeEditorBase() {}

    virtual void setAddressee( const KABC::Addressee& ) = 0;
    virtual const KABC::Addressee &addressee() = 0;

    virtual void setInitialFocus() = 0;

    virtual void contactsSelectionChanged()
    {
       KABC::Addressee::List list = selectedContacts();
       setAddressee( list[ 0 ] );
    }

    virtual void load() = 0;
    virtual void save() = 0;

    virtual bool dirty() = 0;

    bool isExtension() { return mIsExtension; }

    virtual bool readyToClose() { return true; }

  private:
    bool mIsExtension;
};

#endif
