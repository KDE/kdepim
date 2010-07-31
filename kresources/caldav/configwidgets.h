/*=========================================================================
| KCalDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
| (c) 2009  Kumaran Santhanam (initial KDE4 version)
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| Automatic Reload / Automatic Save configuration widgets.
| The code is mostly taken from resourcecachedconfig.h/cpp files from
| the kcal library and changed to meet our requirements.
| The original copyright is below.
 ========================================================================*/
/*
  This file is part of the kcal library.

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
#ifndef KCALDAV_AUTOWIDGETS_H
#define KCALDAV_AUTOWIDGETS_H

#include <tqwidget.h>
#include <kdemacros.h>

namespace KCal {

class ResourceCached;

/**
  Configuration widget for reload policy

  @see ResourceCached
*/
class KDE_EXPORT CalDavReloadConfig : public QWidget
{
  Q_OBJECT
  public:
    explicit CalDavReloadConfig( TQWidget *parent = 0 );
    ~CalDavReloadConfig();
  public slots:
    void loadSettings( ResourceCached *resource );
    void saveSettings( ResourceCached *resource );

  protected slots:
    void slotIntervalToggled( bool );

  private:
    //@cond PRIVATE
    //Q_DISABLE_COPY( CalDavReloadConfig )
    class Private;
    Private *const d;
    //@endcond
};

/**
  Configuration widget for save policy

  @see ResourceCached
*/
class KDE_EXPORT CalDavSaveConfig : public QWidget
{
    Q_OBJECT
  public:
    explicit CalDavSaveConfig( TQWidget *parent = 0 );
    ~CalDavSaveConfig();

  public slots:
    void loadSettings( ResourceCached *resource );
    void saveSettings( ResourceCached *resource );

  protected slots:
    void slotIntervalToggled( bool );

  private:
    //@cond PRIVATE
    //Q_DISABLE_COPY( CalDavSaveConfig )
    class Private;
    Private *const d;
    //@endcond
};

}

#endif // KCALDAV_AUTOWIDGETS_H
