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
