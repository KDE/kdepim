/*
    This file is part of KitchenSync.

    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KSYNC_LOCALKONNECTORCONFIG_H
#define KSYNC_LOCALKONNECTORCONFIG_H

#include <kurlrequester.h>
#include <kresources/configwidget.h>

#include <qwidget.h>

namespace KSync {

class LocalKonnectorConfig : public KRES::ConfigWidget
{
    Q_OBJECT
  public:
    LocalKonnectorConfig( QWidget *parent );
    ~LocalKonnectorConfig();

    void loadSettings( KRES::Resource *resource );
    void saveSettings( KRES::Resource *resource );

  protected slots:
    void selectAddressBookResource();
    void selectCalendarResource();

  private:
    KURLRequester *mCalendarFile;
    KURLRequester *mAddressBookFile;
    KURLRequester *mBookmarkFile;
};

}

#endif
