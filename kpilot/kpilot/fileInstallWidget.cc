/* fileInstallWidget.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This file defines the internal conduit "File Installer"
** that accepts drags of URLs containing Palm DBs, prcs, and
** such. It also does the HotSync part of installing files
** on the Pilot.
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
static const char *fileinstallwidget_id="$Id$";


#include <sys/types.h>
#include <dirent.h>
#include <iostream.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <qdir.h>
#include <qfile.h>
#include <qlist.h>
#include <qlistbox.h>
#include <qstring.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qdragobject.h>

#include <kstddirs.h>
#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kio/netaccess.h>

#include "kpilotlink.h"
#include "fileInstallWidget.moc"

FileInstallWidget::FileInstallWidget( QWidget* parent,
	const QString& path) : 
	PilotComponent(parent,path), 
	fSaveFileList(false) 
    {
    setGeometry(0, 0, parent->geometry().width(), parent->geometry().height());
    QLabel* label = new QLabel(i18n("Files To Install:"), this);
    label->move(20, 10);
    QPushButton* abutton = new QPushButton(i18n("Clear List"), this);
    abutton->move(20,280);
    connect(abutton, SIGNAL(clicked()), this, SLOT(slotClearButton()));
    abutton = new QPushButton(i18n("Add File"), this);
    abutton->move(20,240);
    connect(abutton, SIGNAL(clicked()), this, SLOT(slotAddFile()));
    fListBox = new QListBox(this);
    fListBox->setGeometry(140, 10, 350, 300);
    setAcceptDrops(true);
    }

/**
  * Adds 'fileName' to the pending file list and the list box if using the gui version
  */
void
FileInstallWidget::addFileToLists(const char* fileName)
    {
    char* newFileName;
    
    /* Will be deleted by the ListBox */
    newFileName = new char[strlen(fileName) + 1];
    strcpy(newFileName, fileName);
    fListBox->insertItem(newFileName, -1);
    }

void FileInstallWidget::dragEnterEvent(QDragEnterEvent* event)
{
  event->accept(QUriDrag::canDecode(event));
}
    

void FileInstallWidget::dropEvent(QDropEvent* drop)
    {
      QStrList list;
      QUriDrag::decode(drop, list);

      kdDebug() << "FileInstallWidget::dropEvent() - Got " << list.first() << endl;

      if(list.first() != 0L)
 	getFilesForInstall(list);
    }

void
FileInstallWidget::slotClearButton()
{
  unsigned int i;
  QString dirname = KGlobal::dirs()->saveLocation("data", QString("kpilot/pending_install/"));
  QDir installDir(dirname);
  for(i = 2; i < installDir.count(); i++)
    {
      unlink((dirname + installDir[i]).latin1());
    }
  refreshFileInstallList();
}

void
FileInstallWidget::initialize()
{
  refreshFileInstallList();
}

void
FileInstallWidget::slotAddFile()
    {
    QStrList strList;
    QString fileName = KFileDialog::getOpenFileName();
    if(!fileName.isEmpty())
	{
	strList.append(fileName.latin1());
	getFilesForInstall(strList);
	}
    }

void
FileInstallWidget::getFilesForInstall(QStrList& fileList)
    {
      unsigned int i = 0;
      QString dirname = KGlobal::dirs()->saveLocation("data", QString("kpilot/pending_install/"));
      while (i < fileList.count())
	{
	  KURL srcName = fileList.at(i);
	  KURL destDir(dirname + "/" + srcName.filename());
	  KIO::NetAccess::copy(srcName, destDir);
	  i++;
	  refreshFileInstallList();
	}
    }


void
FileInstallWidget::preHotSync(char* command)
{
  char buffer[10];
  KConfig* config = KGlobal::config();
  config->setGroup(QString());
  if(config->readNumEntry("SyncFiles"))
    {
      sprintf(buffer, "%d\n", KPilotLink::InstallFile);
      strcat(command, buffer);
    }
}

void
FileInstallWidget::postHotSync()
{
  refreshFileInstallList();
}

bool 
FileInstallWidget::saveData()
    {
    return true;
	/* NOTREACHED */
	(void) fileinstallwidget_id;
    }

void 
FileInstallWidget::saveInstallList()
    {
//     QString installList = kapp->localkdedir();
//     installList += KPILOT_INSTALL_FILE;

//     QFile installFile(installList);
//     installFile.open(IO_WriteOnly | IO_Truncate);
//     QTextStream t(&installFile);

//     while(fFileList.first())
// 	{
// 	t << fFileList.first()->url() << endl;
// 	fFileList.remove(fFileList.first());
// 	}
//     installFile.close();
    }


void
FileInstallWidget::refreshFileInstallList()
    {
      unsigned int i;
      fListBox->clear();
      QString dirname = KGlobal::dirs()->saveLocation("data", QString("kpilot/pending_install/"));
      QDir installDir(dirname);
      for(i = 2; i < installDir.count(); i++)
 	addFileToLists(installDir[i].latin1());
    }


// $Log$
// Revision 1.11  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.10  2001/02/05 20:55:07  adridg
// Fixed copyright headers for source releases. No code changed
//
