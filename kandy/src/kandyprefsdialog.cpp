// $Id$

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

#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kfontdialog.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kcolordlg.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include "kandyprefs.h"

#include "kandyprefsdialog.h"
#include "kandyprefsdialog.moc"


KandyPrefsDialog::KandyPrefsDialog(QWidget *parent, char *name, bool modal) :
  KPrefsDialog(KandyPrefs::instance(),parent,name,modal)
{
  setupSerialTab();
  setupWindowsTab();
}


KandyPrefsDialog::~KandyPrefsDialog()
{
}


void KandyPrefsDialog::setupSerialTab()
{
  QFrame *topFrame = addPage(i18n("Serial Interface"),0,
      DesktopIcon("connect_no",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame,6,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());
  
  KPrefsWidString *serialDevice =
      new KPrefsWidString(i18n("Serial Device"),
                          &(KandyPrefs::instance()->mSerialDevice),this,
                          topFrame);
  topLayout->addWidget(serialDevice->label(),0,0);
  topLayout->addWidget(serialDevice->lineEdit(),0,1);

  KPrefsWidBool *openOnStartup = 
       new KPrefsWidBool(i18n("Open Modem On Startup"),
                         &(KandyPrefs::instance()->mStartupModem),this,
                         topFrame);
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
  
  KPrefsWidBool *startupTerminal = 
       new KPrefsWidBool(i18n("Open Terminal Window On Startup"),
                         &(KandyPrefs::instance()->mStartupTerminalWin),this,
                         topFrame);
  topLayout->addWidget(startupTerminal->checkBox(),0,0);
  
  KPrefsWidBool *startupMobile = 
       new KPrefsWidBool(i18n("Open Mobile Window On Startup"),
                         &(KandyPrefs::instance()->mStartupMobileWin),this,
                         topFrame);
  topLayout->addWidget(startupMobile->checkBox(),1,0);
  
  topLayout->setRowStretch(2,1);
}
