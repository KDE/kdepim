// fileInstallWidget.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$


static const char *id="$Id$";


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
#include <kurl.h>
#include <kmessagebox.h>
#include <kwin.h>
#include <kapp.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kio/netaccess.h>

#include "kpilot.h"
#include "kpilotlink.h"
#include "fileInstallWidget.moc"

FileInstallWidget::FileInstallWidget(KPilotInstaller* installer, QWidget* parent)//, QList<KURL>* fileList);
  : PilotComponent(parent), fSaveFileList(false), fKPilotInstaller(installer)//, fHotSyncEnabled(true)
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
    installer->addComponentPage(this, i18n("File Installer"));
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
	(void) id;
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
