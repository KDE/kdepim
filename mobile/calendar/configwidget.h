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

#include "declarativewidgetbase.h"
#include "mainview.h"

#include "calendarviews/eventview.h"
#include "calendarviews/prefs.h"

#include <QGraphicsProxyWidget>
#include <QWidget>

class KComboBox;
class KConfigDialogManager;
class KTimeComboBox;
class QCheckBox;
class Ui_ConfigWidget;

class ConfigWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit ConfigWidget( QWidget *parent = 0 );
    ~ConfigWidget();

    void setPreferences( const EventViews::PrefsPtr &preferences );

  public Q_SLOTS:
    void load();
    void save();

    void setNewTime( int hour, int minute );

  Q_SIGNALS:
    void configChanged();
    void showClockWidget( int hour, int minute );

    void dayBeginsFocus( QObject *object );
    void dailyStartingHourFocus( QObject *object );
    void dailyEndingHourFocus( QObject *object );
    void defaultAppointmentTimeFocus( QObject *object );

  protected:
    bool eventFilter( QObject *object, QEvent *event );

  private Q_SLOTS:
    void showClock( QObject *object );

  private:
    void loadFromExternalSettings();
    void saveToExternalSettings();

    Ui_ConfigWidget *mUi;
    KConfigDialogManager *mManager;
    KComboBox *mHolidayCombo;
    QVector<QCheckBox*> mWorkDays;
    EventViews::PrefsPtr mViewPrefs;
    KTimeComboBox *mFocusedTimeWidget;
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

  public Q_SLOTS:
    void load();
    void save();

    void setNewTime( int hour, int minute );

  Q_SIGNALS:
    void configChanged();
    void showClockWidget( int hour, int minute );
};

#endif
