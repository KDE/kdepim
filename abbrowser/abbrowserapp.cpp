// $Id$

#include <kdebug.h>
#include <kcmdlineargs.h>

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
      if (mAbBrowser->isMinimized()) {
        kdDebug() << "AbBrowser window is minimized." << endl;
        // TODO: Restoring minimized window does not work.
        mAbBrowser->showNormal();
      } else {
        mAbBrowser->raise();
      }
    } else {
      mAbBrowser = new Pab;
      mAbBrowser->show();
    }
    
    if (!addr.isEmpty()) mAbBrowser->addEmail(addr);
  }

  return 0;
}
