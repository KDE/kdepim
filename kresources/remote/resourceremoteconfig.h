/*
    This file is part of libkcal.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_RESOURCEREMOTECONFIG_H
#define KCAL_RESOURCEREMOTECONFIG_H

#include <kresources/resource.h>
#include <kresources/configwidget.h>
#include "remote_export.h"

class KUrlRequester;

namespace KCal {

class ResourceCachedReloadConfig;
class ResourceCachedSaveConfig;

/**
  Configuration widget for remote resource.
  
  @see ResourceRemote
*/
class KCAL_RESOURCEREMOTE_EXPORT ResourceRemoteConfig : public KRES::ConfigWidget
{ 
    Q_OBJECT
  public:
    explicit ResourceRemoteConfig( QWidget *parent = 0 );

  public Q_SLOTS:
    virtual void loadSettings( KRES::Resource *resource );
    virtual void saveSettings( KRES::Resource *resource );

  private:
    KUrlRequester *mDownloadUrl;
    KUrlRequester *mUploadUrl;

    ResourceCachedReloadConfig *mReloadConfig;
    ResourceCachedSaveConfig *mSaveConfig;

    class Private;
    Private *d;
};

}

#endif
