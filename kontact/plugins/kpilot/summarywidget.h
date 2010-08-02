/*
    This file is part of Kontact.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef SUMMARYWIDGET_H
#define SUMMARYWIDGET_H

#include "summary.h"

#include <dcopobject.h>
#include <pilotDaemonDCOP.h>

#include <tqmap.h>
#include <tqpixmap.h>
#include <tqptrlist.h>
#include <tqstringlist.h>
#include <tqtimer.h>
#include <tqwidget.h>
#include <tqdatetime.h>

class TQGridLayout;
class TQLabel;
class KURLLabel;

class SummaryWidget : public Kontact::Summary, public DCOPObject
{
  Q_OBJECT
  K_DCOP

  public:
    SummaryWidget( TQWidget *parent, const char *name = 0 );
    virtual ~SummaryWidget();

    int summaryHeight() const { return 1; }

    TQStringList configModules() const;

  k_dcop:
    // all the information is pushed to Kontact by the daemon, to remove the chance of Kontact calling a daemon
    // that is blocked for some reason, and blocking itself.
    void receiveDaemonStatusDetails( TQDateTime, TQString, TQStringList, TQString, TQString, TQString, bool );
  private slots:
    void updateView();
    void showSyncLog( const TQString &filename );
    void startKPilot();
    void slotAppRemoved( const TQCString & );
  private:
    TQTimer mTimer;

    TQLabel*mSyncTimeTextLabel;
    TQLabel*mSyncTimeLabel;
    KURLLabel*mShowSyncLogLabel;
    TQLabel*mPilotUserTextLabel;
    TQLabel*mPilotUserLabel;
    TQLabel*mPilotDeviceTextLabel;
    TQLabel*mPilotDeviceLabel;
    TQLabel*mDaemonStatusTextLabel;
    TQLabel*mDaemonStatusLabel;
    TQLabel*mConduitsTextLabel;
    TQLabel*mConduitsLabel;
    TQLabel*mNoConnectionLabel;
    KURLLabel*mNoConnectionStartLabel;

    TQGridLayout *mLayout;

    TQDateTime mLastSyncTime;
    TQString mDaemonStatus;
    TQStringList mConduits;
    TQString mSyncLog;
    TQString mUserName;
    TQString mPilotDevice;
    bool mDCOPSuccess;

    bool mStartedDaemon; // Record whether the daemon was started by kontact
    bool mShouldStopDaemon;
};

#endif

