// $Id$

#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kwin.h>

#include "abbrowser.h"

#include "abbrowserapp.h"

AbBrowserApp::AbBrowserApp()
{
  mAbBrowser = 0;
}

AbBrowserApp::~AbBrowserApp()
{
}

int AbBrowserApp::newInstance()
{
  if (isRestored()) {
    // There can only be one main window
    if (KMainWindow::canBeRestored(1)) {
      mAbBrowser = new Pab;
      mAbBrowser->restore(1);
    }
  } else {
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    QCString addrStr = args->getOption("addr");

    QString addr;
    if (!addrStr.isEmpty()) addr = QString::fromLocal8Bit(addrStr);
    
    args->clear();

    if (mAbBrowser) {
      kdDebug() << "AbBrowser already running." << endl;
      KWin::setActiveWindow(mAbBrowser->winId());
    } else {
      mAbBrowser = new Pab;
      mAbBrowser->show();
    }
    
    if (!addr.isEmpty()) mAbBrowser->addEmail(addr);
  }

  return 0;
}
