/*
    This file is part of libkcal.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
		Based on the remote resource:
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KCAL_RESOURCEBLOGGINGCONFIG_H
#define KCAL_RESOURCEBLOGGINGCONFIG_H

#include <kresources/configwidget.h>

class ResourceBloggingSettings;

namespace KCal {

class ResourceCachedReloadConfig;
class ResourceCachedSaveConfig;

/**
  Configuration widget for blogging resource.
  
  @see ResourceBlogging
*/
class ResourceBloggingConfig : public KRES::ConfigWidget
{ 
    Q_OBJECT
  public:
    ResourceBloggingConfig( QWidget *parent = 0, const char *name = 0 );

  public slots:
    virtual void loadSettings( KRES::Resource *resource );
    virtual void saveSettings( KRES::Resource *resource );

  private:
	  ResourceBloggingSettings *mPage;

    ResourceCachedReloadConfig *mReloadConfig;
    ResourceCachedSaveConfig *mSaveConfig;

    class Private;
    Private *d;
};

}

#endif
