/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef INCIDENCEEDITOR_SCHEDULINGDIALOG_H
#define INCIDENCEEDITOR_SCHEDULINGDIALOG_H

#include "incidenceeditors-ng_export.h"
#ifdef KDEPIM_MOBILE_UI
#include "ui_mobileschedulingdialog.h"
#else
#include "ui_schedulingdialog.h"
#endif

#include <KDialog>
#include <KDateTime>
namespace IncidenceEditorNG {

class FreePeriodModel;
class ConflictResolver;
class VisualFreeBusyWidget;

class INCIDENCEEDITORS_NG_EXPORT SchedulingDialog : public KDialog, private Ui_Dialog
{
  Q_OBJECT
  public:
    SchedulingDialog( const QDate &startDate, const QTime &startTime, int duration,
                      ConflictResolver *resolver, QWidget *parent );
    ~SchedulingDialog();

    QDate selectedStartDate() const;
    QTime selectedStartTime() const;

  public slots:
    void slotUpdateIncidenceStartEnd( const KDateTime &startDateTime,
                                      const KDateTime &endDateTime );

  signals:
    void startDateChanged( const QDate &newDate );
    void startTimeChanged( const QTime &newTime );
    void endDateChanged( const QDate &newDate );
    void endTimeChanged( const QTime &newTime );

  private slots:
    void slotWeekdaysChanged();
    void slotMandatoryRolesChanged();
    void slotStartDateChanged( const QDate &newDate );

    void slotRowSelectionChanged( const QModelIndex &current, const QModelIndex &previous );
    void slotSetEndTimeLabel( const QTime &startTime );

  private:
    void updateWeekDays( const QDate &oldDate );
    void fillCombos();

    QDate mStDate;
    QDate mSelectedDate;
    QTime mSelectedTime;
    int mDuration; //!< In seconds

    ConflictResolver *mResolver;
    FreePeriodModel *mPeriodModel;
#ifndef KDEPIM_MOBILE_UI
    VisualFreeBusyWidget *mVisualWidget;
#endif
};

}

#endif
