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
#include <qlistbox.h>
#include <qlist.h>
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
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* drop);

    KPilotInstaller* getPilotInstallerApp() { return fKPilotInstaller; }

    private:
    QListBox*   fListBox;
    QList<KURL> fFileList;
    bool        fSaveFileList;

    KPilotInstaller* fKPilotInstaller;

 protected slots:
    void slotClearButton();
    void slotAddFile();
    };

#endif
