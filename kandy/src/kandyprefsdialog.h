/*
    This file is part of Kandy.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef _KANDYPREFSDIALOG_H
#define _KANDYPREFSDIALOG_H

#include <qframe.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>

#include <kdialogbase.h>

#include <libkdepim/kprefsdialog.h>

/**
  Dialog to change the kandy configuration.
*/
class KandyPrefsDialog : public KPrefsDialog
{
    Q_OBJECT
  public:
    /** Initialize dialog and pages */
    KandyPrefsDialog(QWidget *parent=0,char *name=0,bool modal=false);
    ~KandyPrefsDialog();

  protected:
    void setupSerialTab();
    void setupAddressbookTab();
    void setupWindowsTab();
 private:
    KPrefsWidString *serialDevice;
    KPrefsWidString *lockDir;
    KPrefsWidBool *openOnStartup;
    KPrefsWidBool *autoSetClock;
    
    KPrefsWidBool *excHome;
    KPrefsWidBool *excWork;
    KPrefsWidBool *excMsg;
    KPrefsWidBool *excFax;
    KPrefsWidBool *excCell;
    KPrefsWidBool *excVideo;
    KPrefsWidBool *excBbs;
    KPrefsWidBool *excModem;
    KPrefsWidBool *excCar;
    KPrefsWidBool *excISDN;
    KPrefsWidBool *excPager;
    
    KPrefsWidBool *useHomeSuff;
    KPrefsWidBool *useWorkSuff;
    KPrefsWidBool *useMessagingSuff;
    KPrefsWidBool *useFaxSuff;
    KPrefsWidBool *useCellSuff;
    KPrefsWidBool *useVideoSuff;
    KPrefsWidBool *useMailboxSuff;
    KPrefsWidBool *useModemSuff;
    KPrefsWidBool *useCarSuff;
    KPrefsWidBool *useISDNSuff;
    KPrefsWidBool *usePagerSuff;
    
    KPrefsWidString *HomeSuff;
    KPrefsWidString *WorkSuff;
    KPrefsWidString *MessagingSuff;
    KPrefsWidString *FaxSuff;
    KPrefsWidString *CellSuff;
    KPrefsWidString *VideoSuff;
    KPrefsWidString *MailboxSuff;
    KPrefsWidString *ModemSuff;
    KPrefsWidString *CarSuff;
    KPrefsWidString *ISDNSuff;
    KPrefsWidString *PagerSuff;
    
    KPrefsWidBool *startupTerminal;
    KPrefsWidBool *startupMobile;
    KPrefsWidString *DisplayWidth;
};

#endif
