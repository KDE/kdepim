/* fileInstallWidget.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This file defines the file install widget, which is the thing
** that accepts file drags for later installation into the Pilot.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
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
    FileInstallWidget(QWidget* parent, const QString& dbPath);
    ~FileInstallWidget() { }

    // Pilot Component Methods:
      void initialize();
      void preHotSync(char* command);
      void postHotSync();
    bool saveData();


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


// $Log$
// Revision 1.7  2001/02/06 08:05:19  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
