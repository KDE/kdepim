/*
    This file is part of libkcal.

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
#ifndef KCAL_RESOURCECACHEDCONFIG_H
#define KCAL_RESOURCECACHEDCONFIG_H

#include <qwidget.h>
#include <kdepimmacros.h>

class QButtonGroup;
class QSpinBox;

namespace KCal {

class ResourceCached;

/**
  Configuration widget for reload policy
  
  @see ResourceCached
*/
class KDE_EXPORT ResourceCachedReloadConfig : public QWidget
{ 
    Q_OBJECT
  public:
    ResourceCachedReloadConfig( QWidget *parent = 0, const char *name = 0 );

  public slots:
    void loadSettings( ResourceCached *resource );
    void saveSettings( ResourceCached *resource );

  protected slots:
    void slotIntervalStateChanged( int );

  private:
    QButtonGroup *mGroup;
    QSpinBox *mIntervalSpin;

    class Private;
    Private *d;
};

/**
  Configuration widget for save policy
  
  @see ResourceCached
*/
class KDE_EXPORT ResourceCachedSaveConfig : public QWidget
{ 
    Q_OBJECT
  public:
    ResourceCachedSaveConfig( QWidget *parent = 0, const char *name = 0 );

  public slots:
    void loadSettings( ResourceCached *resource );
    void saveSettings( ResourceCached *resource );

  protected slots:
    void slotIntervalStateChanged( int );

  private:
    QButtonGroup *mGroup;
    QSpinBox *mIntervalSpin;

    class Private;
    Private *d;
};

}

#endif
