
#include "ksync_mainwindow.h"


using namespace KitchenSync;


KSyncMainWindow::KSyncMainWindow(QWidget *widget, const char *name, WFlags f)
  :
  KParts::MainWindow( widget, name, f ){
  setXMLFile("ksyncgui.rc");
  menuBar()->show();
};

KSyncMainWindow::~KSyncMainWindow()
{

}

//#include "ksync_mainwindow.moc"
