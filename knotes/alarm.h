
/*

 $Id$

 KPostit -- postit Notes for the KDE project

 Copyright (C) Bernd Johannes Wuebben
               wuebben@math.cornell.edu
	       wuebben@kde.org

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 */


#ifndef _ALARM_DLG_H_
#define _ALARM_DLG_H_

#include <qgroupbox.h> 
#include <qcheckbox.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qframe.h>
#include <qdatetime.h> 

#include <kcolordlg.h>
#include "knotes.h"
#include <bwdatetime.h>

#include <spin.h>

class AlarmDlg : public QDialog
{
    Q_OBJECT

public:

    AlarmDlg ( KPostit *parent = 0, const char *name=0);

    QDateTime getDateTime();

protected:

    void focusInEvent( QFocusEvent *);
    void resizeEvent ( QResizeEvent *);

private:

    QString pname;
    QDateTime mydatetime;
    QDateTime qdt;
    QPushButton *ok, *cancel;
    QGroupBox 	*frame1;
    KPostit *postit;
    BWDateTime *spins;
    bool alarm_is_on;

public slots:
     
    void cancel_slot();
    void ok_slot();

private:
    bool checkDateTime();
};

#endif
