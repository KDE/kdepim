/*
    This file is part of kdepim.

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef KABC_RESOURCEGROUPWARECONFIG_H
#define KABC_RESOURCEGROUPWARECONFIG_H

#include <kresources/configwidget.h>

#include <qmap.h>

class KComboBox;
class KLineEdit;
class KListView;
class KURLRequester;

namespace KABC {

class ResourceGroupware;

class ResourceGroupwareConfig : public KRES::ConfigWidget
{ 
  Q_OBJECT

  public:
    ResourceGroupwareConfig( QWidget* parent = 0, const char* name = 0 );

  public slots:
    void loadSettings( KRES::Resource* );
    void saveSettings( KRES::Resource* );

  protected slots:
    void updateAddressBookList();

  private:
    void updateAddressBookView();
    void saveAddressBookSettings();
    void loadAddressBookSettings();
    void saveServerSettings( ResourceGroupware *resource );

    KURLRequester *mURL;
    KLineEdit *mUser;
    KLineEdit *mPassword;

    KListView *mAddressBookView;
    KComboBox *mAddressBookBox;

    QStringList mWriteAddressBookIds;

    ResourceGroupware *mResource;
};

}

#endif
