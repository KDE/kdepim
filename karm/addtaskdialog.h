/*
 *   karm
 *   This file only: Copyright (C) 1999  Espen Sand, espensa@online.no
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef ADD_TASK_DIALOG_included
#define ADD_TASK_DIALOG_included

#include <stdlib.h>
#include <kdialogbase.h>
#include <qvalidator.h>
#include <qcheckbox.h>
#include <vector>

#include "taskview.h"       // only for DesktopListType
#include "ktimewidget.h"

class QLineEdit;
class KArmTimeWidget;
class QRadioButton;

/**
 * Dialog to add a new task or edit an existing task.
 */

class AddTaskDialog : public KDialogBase
{
  Q_OBJECT

  public:
    AddTaskDialog(QString caption, bool editDlg, DesktopListType* desktopList=0);
    void setTask(const QString &name, long time, long sessionTime);
    QString taskName() const;

    // return user choices
    void status( long *total, long *totalDiff, 
                 long *session, long *sessionDiff, 
                 DesktopListType *desktopList) const;
    
  private slots:
    void slotAbsolutePressed();
    void slotRelativePressed();
    void slotAutoTrackingPressed();

    void enterWhatsThis();

  private:
    QLineEdit* _name;
    KArmTimeWidget* _totalTW;
    KArmTimeWidget* _sessionTW;
    KArmTimeWidget* _diffTW;
    QComboBox* _operator;
    std::vector<QCheckBox*> _deskBox; // we only need an array, but ISO forbids
                                 // passing an array as a function argument

    long origTotal;
    long origSession;

    QRadioButton *_absoluteRB;
    QRadioButton *_relativeRB;

    QCheckBox *_desktopCB;
    int desktopCount;

    QLabel* _totalLA;
    QLabel* _sessionLA;
};





#endif // ADD_TASK_DIALOG_included

