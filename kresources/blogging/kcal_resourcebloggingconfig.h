/*
    This file is part of kdepim.

    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Based in part on the OGo resource:
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
#ifndef KCALRESOURCEBLOGGINGCONFIG_H
#define KCALRESOURCEBLOGGINGCONFIG_H

#include "folderconfig.h"

#include <kurlrequester.h>

#include <kresources/resource.h>
#include <kresources/configwidget.h>
#include "kcal_groupwareprefs.h"

class QCheckBox;
class KLineEdit;

namespace KCal {

class ResourceCachedReloadConfig;
class ResourceCachedSaveConfig;

/**
  Configuration widget for groupware kioslave resource.
  
  @see KCalOpenGroupware
*/
class ResourceBloggingConfig : public KRES::ConfigWidget
{ 
    Q_OBJECT
  public:
    ResourceBloggingConfig( QWidget *parent = 0, const char *name = 0 );

  public slots:
    virtual void loadSettings( KRES::Resource *resource );
    virtual void saveSettings( KRES::Resource *resource );

  protected slots:
    void updateFolders();

  private:
    KLineEdit *mUrl;
    KLineEdit *mUserEdit;
    KLineEdit *mPasswordEdit;

    ResourceCachedReloadConfig *mReloadConfig;
    ResourceCachedSaveConfig *mSaveConfig;
    KPIM::FolderConfig *mFolderConfig;
};

}

#endif
