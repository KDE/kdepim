/*
  This file is part of the blog resource.

  Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
  Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2007 Mike Arthur <mike@mikearthur.co.uk>

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
#ifndef KCAL_RESOURCEBLOGCONFIG_H
#define KCAL_RESOURCEBLOGCONFIG_H

#include <kresources/resource.h>
#include <kresources/configwidget.h>
#include "blog_export.h"

class KUrlRequester;
class KLineEdit;
class KComboBox;

namespace KCal
{

class ResourceCachedReloadConfig;
class ResourceCachedSaveConfig;

/**
  Configuration widget for blog resource.

  @see ResourceBlog
*/
class ResourceBlogConfig : public KRES::ConfigWidget
{
    Q_OBJECT
  public:
    ResourceBlogConfig( QWidget *parent = 0 );

  public Q_SLOTS:
    virtual void loadSettings( KRES::Resource *resource );
    virtual void saveSettings( KRES::Resource *resource );

  private:
    KUrlRequester *mUrl;
    KLineEdit *mUser;
    KLineEdit *mPassword;
    KComboBox *mAPI;

    ResourceCachedReloadConfig *mReloadConfig;

    class Private;
    Private *d;
};

}

#endif
