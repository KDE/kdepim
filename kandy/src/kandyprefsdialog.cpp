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

#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qfont.h>
#include <qslider.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qcombobox.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qdatetime.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kfontdialog.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kcolordialog.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include "kandyprefs.h"

#include "kandyprefsdialog.h"
#include "kandyprefsdialog.moc"


KandyPrefsDialog::KandyPrefsDialog(QWidget *parent, char *name, bool modal) :
  KPrefsDialog(KandyPrefs::self(),parent,name,modal)
{
  setupSerialTab();
  setupAddressbookTab();
  setupWindowsTab();
}


KandyPrefsDialog::~KandyPrefsDialog()
{
  delete serialDevice;
  delete lockDir;
  delete openOnStartup;
  delete startupTerminal;
  delete startupMobile;
}

void KandyPrefsDialog::setupSerialTab()
{
  QFrame *topFrame = addPage(i18n("Serial Interface"),0,
      DesktopIcon("connect_no",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame,6,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());
  
  serialDevice = addWidString( KandyPrefs::self()->serialDeviceItem(),
                               topFrame );
  topLayout->addWidget(serialDevice->label(),0,0);
  topLayout->addWidget(serialDevice->lineEdit(),0,1);
  
  lockDir = addWidString( KandyPrefs::self()->lockDirectoryItem(),
                               topFrame );
  topLayout->addWidget(lockDir->label(),1,0);
  topLayout->addWidget(lockDir->lineEdit(),1,1);

  openOnStartup = addWidBool( KandyPrefs::self()->startupModemItem(),
                              topFrame );
  topLayout->addWidget(openOnStartup->checkBox(),2,0);

  autoSetClock = addWidBool( KandyPrefs::self()->autoSetClockItem(),
                             topFrame );
  topLayout->addWidget(autoSetClock->checkBox(),3,0);
  
  topLayout->setRowStretch(4,1);
}

void KandyPrefsDialog::setupAddressbookTab()
{
  QFrame *topFrame = addPage(i18n("Address Book"), 0,
    DesktopIcon("kaddressbook", KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame, 13, 4);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());
  
  excHome = addWidBool ( KandyPrefs::self()->excludeHomeItem(), topFrame );
  topLayout->addWidget(excHome->checkBox(), 0, 0);
  
  excWork = addWidBool ( KandyPrefs::self()->excludeWorkItem(), topFrame );
  topLayout->addWidget(excWork->checkBox(), 1, 0);
  
  excMsg = addWidBool ( KandyPrefs::self()->excludeMessagingItem(), topFrame );
  topLayout->addWidget(excMsg->checkBox(), 2, 0);
  
  excFax = addWidBool ( KandyPrefs::self()->excludeFaxItem(), topFrame );
  topLayout->addWidget(excFax->checkBox(), 3, 0);
  
  excCell = addWidBool ( KandyPrefs::self()->excludeCellItem(), topFrame );
  topLayout->addWidget(excCell->checkBox(), 4, 0);
  
  excVideo = addWidBool ( KandyPrefs::self()->excludeVideoItem(), topFrame );
  topLayout->addWidget(excVideo->checkBox(), 5, 0);
  
  excBbs = addWidBool ( KandyPrefs::self()->excludeMailboxItem(), topFrame );
  topLayout->addWidget(excBbs->checkBox(), 6, 0);
  
  excModem = addWidBool ( KandyPrefs::self()->excludeModemItem(), topFrame );
  topLayout->addWidget(excModem->checkBox(), 7, 0);
  
  excCar = addWidBool ( KandyPrefs::self()->excludeCarItem(), topFrame );
  topLayout->addWidget(excCar->checkBox(), 8, 0);
  
  excISDN = addWidBool ( KandyPrefs::self()->excludeISDNItem(), topFrame );
  topLayout->addWidget(excISDN->checkBox(), 9, 0);
  
  excPager = addWidBool ( KandyPrefs::self()->excludePagerItem(), topFrame );
  topLayout->addWidget(excPager->checkBox(), 10, 0);

  
  useHomeSuff = addWidBool ( KandyPrefs::self()->useHomeSuffItem(), topFrame );
  topLayout->addWidget(useHomeSuff->checkBox(), 0, 1);
  
  useWorkSuff = addWidBool ( KandyPrefs::self()->useWorkSuffItem(), topFrame );
  topLayout->addWidget(useWorkSuff->checkBox(), 1, 1);
  
  useMessagingSuff = addWidBool ( KandyPrefs::self()->useMessagingSuffItem(), topFrame );
  topLayout->addWidget(useMessagingSuff->checkBox(), 2, 1);
  
  useFaxSuff = addWidBool ( KandyPrefs::self()->useFaxSuffItem(), topFrame );
  topLayout->addWidget(useFaxSuff->checkBox(), 3, 1);
  
  useCellSuff = addWidBool ( KandyPrefs::self()->useCellSuffItem(), topFrame );
  topLayout->addWidget(useCellSuff->checkBox(), 4, 1);
  
  useVideoSuff = addWidBool ( KandyPrefs::self()->useVideoSuffItem(), topFrame );
  topLayout->addWidget(useVideoSuff->checkBox(), 5, 1);
  
  useMailboxSuff = addWidBool ( KandyPrefs::self()->useMailboxSuffItem(), topFrame );
  topLayout->addWidget(useMailboxSuff->checkBox(), 6, 1);
  
  useModemSuff = addWidBool ( KandyPrefs::self()->useModemSuffItem(), topFrame );
  topLayout->addWidget(useModemSuff->checkBox(), 7, 1);
  
  useCarSuff = addWidBool ( KandyPrefs::self()->useCarSuffItem(), topFrame );
  topLayout->addWidget(useCarSuff->checkBox(), 8, 1);
  
  useISDNSuff = addWidBool ( KandyPrefs::self()->useISDNSuffItem(), topFrame );
  topLayout->addWidget(useISDNSuff->checkBox(), 9, 1);
  
  usePagerSuff = addWidBool ( KandyPrefs::self()->usePagerSuffItem(), topFrame );
  topLayout->addWidget(usePagerSuff->checkBox(), 10, 1);

  
  HomeSuff = addWidString( KandyPrefs::self()->homeSuffItem(), topFrame );
  topLayout->addWidget(HomeSuff->label(), 0, 2);
  topLayout->addWidget(HomeSuff->lineEdit(), 0, 3);
  
  WorkSuff = addWidString( KandyPrefs::self()->workSuffItem(), topFrame );
  topLayout->addWidget(WorkSuff->label(), 1, 2);
  topLayout->addWidget(WorkSuff->lineEdit(), 1, 3);
  
  MessagingSuff = addWidString( KandyPrefs::self()->messagingSuffItem(), topFrame );
  topLayout->addWidget(MessagingSuff->label(), 2, 2);
  topLayout->addWidget(MessagingSuff->lineEdit(), 2, 3);
  
  FaxSuff = addWidString( KandyPrefs::self()->faxSuffItem(), topFrame );
  topLayout->addWidget(FaxSuff->label(), 3, 2);
  topLayout->addWidget(FaxSuff->lineEdit(), 3, 3);
  
  CellSuff = addWidString( KandyPrefs::self()->cellSuffItem(), topFrame );
  topLayout->addWidget(CellSuff->label(), 4, 2);
  topLayout->addWidget(CellSuff->lineEdit(), 4, 3);
  
  VideoSuff = addWidString( KandyPrefs::self()->videoSuffItem(), topFrame );
  topLayout->addWidget(VideoSuff->label(), 5, 2);
  topLayout->addWidget(VideoSuff->lineEdit(), 5, 3);
  
  MailboxSuff = addWidString( KandyPrefs::self()->mailboxSuffItem(), topFrame );
  topLayout->addWidget(MailboxSuff->label(), 6, 2);
  topLayout->addWidget(MailboxSuff->lineEdit(), 6, 3);
  
  ModemSuff = addWidString( KandyPrefs::self()->modemSuffItem(), topFrame );
  topLayout->addWidget(ModemSuff->label(), 7, 2);
  topLayout->addWidget(ModemSuff->lineEdit(), 7, 3);
  
  CarSuff = addWidString( KandyPrefs::self()->carSuffItem(), topFrame );
  topLayout->addWidget(CarSuff->label(), 8, 2);
  topLayout->addWidget(CarSuff->lineEdit(), 8, 3);
  
  ISDNSuff = addWidString( KandyPrefs::self()->iSDNSuffItem(), topFrame );
  topLayout->addWidget(ISDNSuff->label(), 9, 2);
  topLayout->addWidget(ISDNSuff->lineEdit(), 9, 3);
  
  PagerSuff = addWidString( KandyPrefs::self()->pagerSuffItem(), topFrame );
  topLayout->addWidget(PagerSuff->label(), 10, 2);
  topLayout->addWidget(PagerSuff->lineEdit(), 10, 3);


  topLayout->setRowStretch(11, 1);
  
  if ( !KandyPrefs::self()->useHomeSuff() )
  {
    HomeSuff->lineEdit()->setEnabled( false );
    HomeSuff->label()->setEnabled( false );
  }
  if ( !KandyPrefs::self()->useWorkSuff() )
  {
    WorkSuff->lineEdit()->setEnabled( false );
    WorkSuff->label()->setEnabled( false );
  }
  if ( !KandyPrefs::self()->useMessagingSuff() )
  {
    MessagingSuff->lineEdit()->setEnabled( false );
    MessagingSuff->label()->setEnabled( false );
  }
  if ( !KandyPrefs::self()->useFaxSuff() )
  {
    FaxSuff->lineEdit()->setEnabled( false );
    FaxSuff->label()->setEnabled( false );
  }
  if ( !KandyPrefs::self()->useCellSuff() )
  {
    CellSuff->lineEdit()->setEnabled( false );
    CellSuff->label()->setEnabled( false );
  }
  if ( !KandyPrefs::self()->useVideoSuff() )
  {
    VideoSuff->lineEdit()->setEnabled( false );
    VideoSuff->label()->setEnabled( false );
  }
  if ( !KandyPrefs::self()->useMailboxSuff() )
  {
    MailboxSuff->lineEdit()->setEnabled( false );
    MailboxSuff->label()->setEnabled( false );
  }
  if ( !KandyPrefs::self()->useModemSuff() )
  {
    ModemSuff->lineEdit()->setEnabled( false );
    ModemSuff->label()->setEnabled( false );
  }
  if ( !KandyPrefs::self()->useCarSuff() )
  {
    CarSuff->lineEdit()->setEnabled( false );
    CarSuff->label()->setEnabled( false );
  }
  if ( !KandyPrefs::self()->useISDNSuff() )
  {
    ISDNSuff->lineEdit()->setEnabled( false );
    ISDNSuff->label()->setEnabled( false );
  }
  if ( !KandyPrefs::self()->usePagerSuff() )
  {
    PagerSuff->lineEdit()->setEnabled( false );
    PagerSuff->label()->setEnabled( false );
  }
  
  
  connect( useHomeSuff->checkBox(), SIGNAL(toggled(bool)),
           HomeSuff->lineEdit(), SLOT(setEnabled(bool)) );
  connect( useHomeSuff->checkBox(), SIGNAL(toggled(bool)),
           HomeSuff->label(), SLOT(setEnabled(bool)) );
  
  connect( useWorkSuff->checkBox(), SIGNAL(toggled(bool)),
           WorkSuff->lineEdit(), SLOT(setEnabled(bool)) );
  connect( useWorkSuff->checkBox(), SIGNAL(toggled(bool)),
           WorkSuff->label(), SLOT(setEnabled(bool)) );
  
  connect( useMessagingSuff->checkBox(), SIGNAL(toggled(bool)),
           MessagingSuff->lineEdit(), SLOT(setEnabled(bool)) );
  connect( useMessagingSuff->checkBox(), SIGNAL(toggled(bool)),
           MessagingSuff->label(), SLOT(setEnabled(bool)) );
  
  connect( useFaxSuff->checkBox(), SIGNAL(toggled(bool)),
           FaxSuff->lineEdit(), SLOT(setEnabled(bool)) );
  connect( useFaxSuff->checkBox(), SIGNAL(toggled(bool)),
           FaxSuff->label(), SLOT(setEnabled(bool)) );
  
  connect( useCellSuff->checkBox(), SIGNAL(toggled(bool)),
           CellSuff->lineEdit(), SLOT(setEnabled(bool)) );
  connect( useCellSuff->checkBox(), SIGNAL(toggled(bool)),
           CellSuff->label(), SLOT(setEnabled(bool)) );
  
  connect( useVideoSuff->checkBox(), SIGNAL(toggled(bool)),
           VideoSuff->lineEdit(), SLOT(setEnabled(bool)) );
  connect( useVideoSuff->checkBox(), SIGNAL(toggled(bool)),
           VideoSuff->label(), SLOT(setEnabled(bool)) );
  
  connect( useMailboxSuff->checkBox(), SIGNAL(toggled(bool)),
           MailboxSuff->lineEdit(), SLOT(setEnabled(bool)) );
  connect( useMailboxSuff->checkBox(), SIGNAL(toggled(bool)),
           MailboxSuff->label(), SLOT(setEnabled(bool)) );
  
  connect( useModemSuff->checkBox(), SIGNAL(toggled(bool)),
           ModemSuff->lineEdit(), SLOT(setEnabled(bool)) );
  connect( useModemSuff->checkBox(), SIGNAL(toggled(bool)),
           ModemSuff->label(), SLOT(setEnabled(bool)) );
  
  connect( useCarSuff->checkBox(), SIGNAL(toggled(bool)),
           CarSuff->lineEdit(), SLOT(setEnabled(bool)) );
  connect( useCarSuff->checkBox(), SIGNAL(toggled(bool)),
           CarSuff->label(), SLOT(setEnabled(bool)) );
  
  connect( useISDNSuff->checkBox(), SIGNAL(toggled(bool)),
           ISDNSuff->lineEdit(), SLOT(setEnabled(bool)) );
  connect( useISDNSuff->checkBox(), SIGNAL(toggled(bool)),
           ISDNSuff->label(), SLOT(setEnabled(bool)) );
  
  connect( usePagerSuff->checkBox(), SIGNAL(toggled(bool)),
           PagerSuff->lineEdit(), SLOT(setEnabled(bool)) );
  connect( usePagerSuff->checkBox(), SIGNAL(toggled(bool)),
           PagerSuff->label(), SLOT(setEnabled(bool)) );
}

void KandyPrefsDialog::setupWindowsTab()
{
  QFrame *topFrame = addPage(i18n("Windows"),0,
      DesktopIcon("window_list",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame,6,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());
  
  startupTerminal = addWidBool( KandyPrefs::self()->startupTerminalWinItem(),
                                topFrame);
  topLayout->addWidget(startupTerminal->checkBox(),0,0);
  
  startupMobile = addWidBool( KandyPrefs::self()->startupMobileWinItem(),
                              topFrame );
  topLayout->addWidget(startupMobile->checkBox(),1,0);
  
  topLayout->setRowStretch(2, 1);
}
