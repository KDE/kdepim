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
  setupWindowsTab();
}


KandyPrefsDialog::~KandyPrefsDialog()
{
  delete serialDevice;
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

  openOnStartup = addWidBool( KandyPrefs::self()->startupModemItem(),
                              topFrame );
  topLayout->addWidget(openOnStartup->checkBox(),1,0);
  
  topLayout->setRowStretch(2,1);
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
  
  topLayout->setRowStretch(2,1);
}
