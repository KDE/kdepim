/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __FILE_INSTALL_WIDGET_H
#define __FILE_INSTALL_WIDGET_H

#include "pilotComponent.h"
#include <kfm.h>
#include <qlistbox.h>
#include <qlist.h>
#include <drag.h>
#include <kurl.h>

class KPilotInstaller;

class FileInstallWidget : public PilotComponent
    {
    Q_OBJECT
    
    public:
    FileInstallWidget(KPilotInstaller* installer, QWidget* parent);//, QList<KURL>* fileList);
    ~FileInstallWidget() { }

    // Pilot Component Methods:
      void initialize();
      void preHotSync(char* command);
      void postHotSync();
//     bool doHotSync(KPilotLink* pilotLink);
    bool saveData();
//     bool hotSyncNeeded();
    //    void enableHotSync(bool yesno) { fHotSyncEnabled = yesno; }


    void refreshFileInstallList();

    signals:
    void fileInstallWidgetDone();

    protected:
    void addFileToLists(const char* fileName);
    void setSaveFileList(bool saveIt) { fSaveFileList = saveIt; }
    bool getSaveFileList() { return fSaveFileList; }
    void getFilesForInstall(QStrList& fileList);
    void saveInstallList();

    KPilotInstaller* getPilotInstallerApp() { return fKPilotInstaller; }

    private:
    QListBox*   fListBox;
    QList<KURL> fFileList;
    KFM*        fKFM;
    bool        fSaveFileList;

    void initKFM();
    KFM* getKFM() { return fKFM; }
    void freeKFM() { if(fKFM) delete fKFM; fKFM = 0L; }

    KPilotInstaller* fKPilotInstaller;
//     bool               fHotSyncEnabled;

 protected slots:
    void kfmFileCopyComplete();
    void slotDropEvent(KDNDDropZone* drop);
    void slotClearButton();
    void slotAddFile();
    };

#endif
