/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>

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
*/
#ifndef KABC_RESOURCEGROUPWAREBASECONFIG_H
#define KABC_RESOURCEGROUPWAREBASECONFIG_H

#include <kresources/configwidget.h>

class KLineEdit;
class KURLRequester;

namespace KPIM {
class FolderConfig;
}

namespace KABC {

class ResourceGroupwareBase;

class ResourceGroupwareBaseConfig : public KRES::ConfigWidget
{ 
  Q_OBJECT

  public:
    ResourceGroupwareBaseConfig( QWidget* parent = 0, const char* name = 0 );

  public slots:
    void loadSettings( KRES::Resource* );
    void saveSettings( KRES::Resource* );

  protected slots:
    void updateFolders();

  private:
    KURLRequester *mURL;
    KLineEdit *mUser;
    KLineEdit *mPassword;
    KPIM::FolderConfig *mFolderConfig;

    ResourceGroupwareBase *mResource;
};

}

#endif
