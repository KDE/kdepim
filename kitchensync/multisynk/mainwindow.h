#ifndef KSYNC_MAINWINDOW_H
#define KSYNC_MAINWINDOW_H

#include <kmainwindow.h>

#include <syncer.h>

class KonnectorPairManager;
class KonnectorPairView;
class LogDialog;

namespace KSync {
class Engine;
}

namespace KPIM {
class StatusbarProgressWidget;
class ProgressDialog;
}

using KPIM::StatusbarProgressWidget;
using KPIM::ProgressDialog;

class MainWindow : public KMainWindow
{
  Q_OBJECT

  public:
    MainWindow( QWidget *widget = 0, const char *name = 0 );
    ~MainWindow();

  private slots:
    void addPair();
    void editPair();
    void deletePair();
    void showLog();
    void startSync();
    void syncDone();

  private:
    void initGUI();

    KonnectorPairManager *mManager;
    KonnectorPairView *mView;
    KSync::Engine *mEngine;
    LogDialog *mLogDialog;
};

#endif
