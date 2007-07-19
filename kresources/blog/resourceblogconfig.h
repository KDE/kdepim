/*
  This file is part of the blog resource.

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

/**
  The class provides a configuration widget for blog resource.

  @see ResourceBlog
*/
class ResourceBlogConfig : public KRES::ConfigWidget
{
  Q_OBJECT
  public:
    /**
      Creates a configuration widget for blog resource.

      @param parent The parent widget to attach to.
    */
    ResourceBlogConfig( QWidget *parent = 0 );

  public Q_SLOTS:
    /**
      Loads the settings for the blog resource for the widget's defaults.

      @param resource The ResourceBlog object.
    */
    void loadSettings( KRES::Resource *resource );

    /**
      Saves the entered settings from the widget to the blog resource.

      @param resource The ResourceBlog object.
    */
    void saveSettings( KRES::Resource *resource );

    /**
      Saves the entered settings from the widget to the blog resource.

      @param id The unique ID for the blog to post to.
      @param name The name for the blog to post to.
    */
    void slotBlogInfoRetrieved( const QString &id, const QString &name );

  private:
    /**
      Used to enter the URL used for XML-RPC access to the blog.
    */
    KUrlRequester *mUrl;

    /**
      Used to enter the username for the blog's XML-RPC authentication.
    */
    KLineEdit *mUser;

    /**
      Used to enter the password for the blog's XML-RPC authentication.
    */
    KLineEdit *mPassword;

    /**
      The combobox used to select the XML-RPC API used to access the blog.
    */
    KComboBox *mAPI;

    /**
      The combobox used to select the blog to post to.
    */
    KComboBox *mBlogs;

    /**
      A widget to configure the cache reload options.
    */
    ResourceCachedReloadConfig *mReloadConfig;
};

}

#endif
