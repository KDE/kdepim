/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
    Author: Kevin Krammer, krake@kdab.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include <QtGui/QMainWindow>

namespace Akonadi
{
  class Calendar;
  class ChangeRecorder;
  class IncidenceChanger;
}

namespace EventViews
{
  class EventView;
}

class QAction;
class Settings;

class MainWindow : public QMainWindow
{
  Q_OBJECT
  public:
    explicit MainWindow( const QStringList &viewNames );

    ~MainWindow();

  private:
    const QStringList mViewNames;

    Ui_MainWindow mUi;

    Akonadi::ChangeRecorder *mChangeRecorder;

    Akonadi::Calendar *mCalendar;

    Akonadi::IncidenceChanger *mIncidenceChanger;

    Settings *mSettings;

  private:
    void addView( const QString &viewName );

  private Q_SLOTS:
    void delayedInit();

    void addViewTriggered( QAction *action );
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
