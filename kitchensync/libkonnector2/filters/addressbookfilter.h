/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef ADDRESSBOOKFILTER_H
#define ADDRESSBOOKFILTER_H

#include "filter.h"

class KListView;

namespace KSync {

class AddressBookSyncee;

class AddressBookConfigWidget : public QWidget
{
  public:
    AddressBookConfigWidget( QWidget *parent, const char *name );

    void setCategories( const QStringList &categories );

    void setSelectedCategories( const QStringList &categories );

    QStringList selectedCategories() const;

  private:
    KListView *mView;
};


class AddressBookFilter : public Filter
{
  public:
    AddressBookFilter( QObject *parent );
    virtual ~AddressBookFilter();
  
    virtual bool supports( Syncee *syncee );
    virtual QWidget *configWidget( QWidget *parent );
    virtual void configWidgetClosed( QWidget *widget );

    virtual void convert( Syncee* );
    virtual void reconvert( Syncee* );

    QString type() const { return "addressbook"; }
  
  private:
    void doLoad();
    void doSave();

    void filterSyncee( AddressBookSyncee*, const QStringList& );
    void unfilterSyncee( AddressBookSyncee* );

    AddressBookSyncEntry::PtrList mFilteredEntries;
    QStringList mSelectedCategories;
};

}

#endif
