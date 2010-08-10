/*
    This file is part of KOrganizer.

    Copyright (C) 2010  Bertjan Broeksema <broeksema@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef INCIDENCEGENERALEDITOR_H
#define INCIDENCEGENERALEDITOR_H

#include <kcalcore/incidence.h>

#include <boost/shared_ptr.hpp>

#include <QtCore/QDateTime>
#include <QtGui/QWidget>

#include <KDateTime>

#include <kcalcore/alarm.h>
#include <kcalcore/event.h>
#include <kcalcore/todo.h>

class QCheckBox;
class QDateTime;


namespace KPIM {
class KTimeZoneComboBox;
}

namespace Ui {
class IncidenceGeneral;
}

class IncidenceGeneralEditor : public QWidget
{
  Q_OBJECT
  public:
    ~IncidenceGeneralEditor();

  public slots:
    void setDuration();

  signals:
    void dateTimeStrChanged( const QString &dateTimeStr );

  protected: /// Methods
    /**
     * Disable creation of plain IncidenceGeneralEditor widgets. Use one of the
     * sub-classes instead.
     */
    explicit IncidenceGeneralEditor( QWidget *parent = 0 );

    void emitDateTimeStr();
    void enableAlarmEditor( bool enable );

    void load( const KCalCore::Incidence::Ptr &incidence );

  protected slots:
    virtual void enableTimeEditors( bool enabled ) = 0;
    virtual void slotHasTimeCheckboxToggled( bool checked );

  protected:
    virtual bool setAlarmOffset( const KCalCore::Alarm::Ptr &alarm, int value ) const = 0;

  private slots:
    void editAlarms();
    void enableRichTextDescription( bool enable );
    void updateAlarmWidgets();
    void updateDefaultAlarmTime();
    void updateRecurrenceSummary( const KCalCore::Incidence::Ptr &incidence );


  private: /// Methods
    KCalCore::Alarm::Ptr alarmFromSimplePage() const;
    void initDescriptionToolBar();
    void setDescription( const QString &text, bool isRich );

  protected: /// Members
    enum AlarmStackPages {
      SimpleAlarmPage,
      AdvancedAlarmLabel
    };

    KCalCore::Incidence::Ptr mIncidence;

    KCalCore::Alarm::List     mAlarmList;
    KCalCore::ICalTimeZones  *mTimeZones;

    Ui::IncidenceGeneral *mUi;
    QCheckBox            *mRichTextCheck;

    // current start and end date and time
    QDateTime mCurrStartDateTime;
    QDateTime mCurrEndDateTime;

    // specs
    KDateTime::Spec mStartSpec;
    KDateTime::Spec mEndSpec;
};

class EventGeneralEditor : public IncidenceGeneralEditor
{
  Q_OBJECT
  public:
    explicit EventGeneralEditor( QWidget *parent = 0 );

    void load( const KCalCore::Event::Ptr &event );

  signals:
    void allDayChanged( bool changed );


  private slots:
    virtual void slotHasTimeCheckboxToggled( bool checked );

  private:
    void enableTimeEditors( bool enabled );
    virtual bool setAlarmOffset( const KCalCore::Alarm::Ptr &alarm, int value ) const;

  private:
//     KCalCore::Todo::Ptr mEvent;
};

class TodoGeneralEditor : public IncidenceGeneralEditor
{
  Q_OBJECT
  public:
    explicit TodoGeneralEditor( QWidget *parent = 0 );

    void load( const KCalCore::Todo::Ptr &todo,
               const QDate &date,
               bool tmpl );

  private slots:
    void completedChanged(int);
    void enableTimeEditors( bool enabled );
    void enableStartEdit( bool enable );
    void enableEndEdit( bool enable );
    void setCompletedDate();
    void updateHasTimeCheckBox();

  private:
    void enableTimeEdit( QWidget *dateEdit,
                         QWidget *timeEdit,
                         KPIM::KTimeZoneComboBox *timeZoneCmb,
                         bool enable );
    virtual bool setAlarmOffset( const KCalCore::Alarm::Ptr &alarm, int value ) const;


  private:
//     KCalCore::Todo::Ptr mTodo;
    bool mAlreadyComplete;
    QDateTime mCompleted;
};

#endif // INCIDENCEGENERALEDITOR_H
