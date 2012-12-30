/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Tobias Koenig <tobias.koenig@kdab.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include "calendarviews/eventview.h"
#include "calendarviews/prefs.h"

#include "declarativewidgetbase.h"
#include "mainview.h"

#include <QGraphicsProxyWidget>
#include <QWidget>

class KConfigDialogManager;

class ConfigWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit ConfigWidget( QWidget *parent = 0 );

    void setPreferences( const EventViews::PrefsPtr &preferences );

  public Q_SLOTS:
    void load();
    void save();

  private:
    void loadFromExternalSettings();
    void saveToExternalSettings();

    KConfigDialogManager *mManager;
    EventViews::PrefsPtr mViewPrefs;
};

class DeclarativeConfigWidget :
#ifndef Q_MOC_RUN
public DeclarativeWidgetBase<ConfigWidget, MainView, &MainView::setConfigWidget>
#else
public QGraphicsProxyWidget
#endif
{
  Q_OBJECT

  public:
    explicit DeclarativeConfigWidget( QGraphicsItem *parent = 0 );
    ~DeclarativeConfigWidget();

    void setPreferences( const EventViews::PrefsPtr &preferences );

  public Q_SLOTS:
    void load();
    void save();
};

#endif
