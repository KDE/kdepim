/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KCAL_CACHESETTINGSDLG_H
#define KCAL_CACHESETTINGSDLG_H

#include <kdialogbase.h>
#include <kdepimmacros.h>


namespace KRES {
class Resource;
}

namespace KCal {

class ResourceCachedReloadConfig;
class ResourceCachedSaveConfig;

/**
  Configuration widget for groupware kioslave resource.
  @see KCalOpenGroupware
*/
class KDE_EXPORT CacheSettingsDialog : public KDialogBase
{ 
    Q_OBJECT
  public:
    CacheSettingsDialog( QWidget *parent = 0, const char *name = 0 );

  public slots:
    virtual void loadSettings( KRES::Resource *resource );
    virtual void saveSettings( KRES::Resource *resource );

  private:
    KCal::ResourceCachedReloadConfig *mReloadConfig;
    KCal::ResourceCachedSaveConfig *mSaveConfig;
};

}

#endif
