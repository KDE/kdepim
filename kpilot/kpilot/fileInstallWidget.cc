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
#include <kurl.h>
#include <kmsgbox.h>
#include <kwm.h>
#include <kapp.h>
#include <kfiledialog.h>
#include "kpilot.h"
#include "kpilotlink.h"
#include "fileInstallWidget.moc"

#define KPILOT_INSTALL_FILE "/share/apps/kpilot/kpilot_install_list"
#define KPILOT_INSTALL_DIR "/share/apps/kpilot/pending_install/"

FileInstallWidget::FileInstallWidget(KPilotInstaller* installer, QWidget* parent)//, QList<KURL>* fileList);
  : PilotComponent(parent), fKFM(0L), fSaveFileList(false), fKPilotInstaller(installer)//, fHotSyncEnabled(true)
    {
    setGeometry(0, 0, parent->geometry().width(), parent->geometry().height());
    QLabel* label = new QLabel(klocale->translate("Files To Install:"), this);
    label->move(20, 10);
    QPushButton* abutton = new QPushButton(klocale->translate("Clear List"), this);
    abutton->move(20,280);
    connect(abutton, SIGNAL(clicked()), this, SLOT(slotClearButton()));
    abutton = new QPushButton(klocale->translate("Add File"), this);
    abutton->move(20,240);
    connect(abutton, SIGNAL(clicked()), this, SLOT(slotAddFile()));
    fListBox = new QListBox(this);
    fListBox->setGeometry(140, 10, 350, 300);
    KDNDDropZone* dropZone = new KDNDDropZone(fListBox, DndURL);
    connect(dropZone, SIGNAL(dropAction(KDNDDropZone*)), this, SLOT(slotDropEvent(KDNDDropZone*)));
    installer->addComponentPage(this, klocale->translate("File Installer"));
    }

/**
  * Adds 'url' to the pending file list and the list box if using the gui version
  */
void
// FileInstallWidget::addFileToLists(KURL* url)
FileInstallWidget::addFileToLists(const char* fileName)
    {
    char* newFileName;
    
    /* Will be deleted by the ListBox */
    newFileName = new char[strlen(fileName) + 1];
    strcpy(newFileName, fileName);
    fListBox->insertItem(newFileName, -1);
    }

void
FileInstallWidget::kfmFileCopyComplete()
    {
    delete fKFM;
    fKFM = 0L;
    if(getPilotInstallerApp()->getQuitAfterCopyComplete())
 	emit fileInstallWidgetDone();
    }
    
void
FileInstallWidget::slotDropEvent(KDNDDropZone* drop)
    {
    QStrList & list = drop->getURLList();

    if(list.first() != 0L)
	getFilesForInstall(list);
    }

void
FileInstallWidget::slotClearButton()
{
  unsigned int i;
  QDir installDir(kapp->localkdedir() + KPILOT_INSTALL_DIR);
  for(i = 2; i < installDir.count(); i++)
    {
      unlink((kapp->localkdedir() + KPILOT_INSTALL_DIR + installDir[i]).data());
    }
  refreshFileInstallList();
}

void
FileInstallWidget::initKFM()
    {
    if(fKFM == 0L)
	fKFM = new KFM;
    fKFM->allowKFMRestart(true);
    if ( !fKFM->isOK() )
	{
	KMsgBox::message(this, klocale->translate("Error"), 
			 klocale->translate("Could not start KFM"), KMsgBox::STOP);
	delete fKFM;
	exit(0);
	}
    }

void
FileInstallWidget::initialize()
{
  unsigned int i;

  getPilotInstallerApp()->testDir( kapp->localkdedir() + "/share/apps/kpilot" );
  getPilotInstallerApp()->testDir( kapp->localkdedir() + KPILOT_INSTALL_DIR);
  fListBox->clear();
  QDir installDir(kapp->localkdedir() + KPILOT_INSTALL_DIR);
  for(i = 2; i < installDir.count(); i++)
    addFileToLists(installDir[i]);
}

void
FileInstallWidget::slotAddFile()
    {
    QStrList strList;
    QString fileName = KFileDialog::getOpenFileName();
    if(fileName != NULL)
	{
	strList.append(fileName);
	getFilesForInstall(strList);
	}
    }

void
FileInstallWidget::getFilesForInstall(QStrList& fileList)
    {
    QString tempFileName = '\0';
    QString destFileDir = '\0';
    unsigned int i = 0;

    initKFM(); // Init fKFM

    destFileDir = "file:";
    destFileDir += kapp->localkdedir();
    destFileDir += KPILOT_INSTALL_DIR;

    while(i < fileList.count())
	{
	/* Will be deleted by list: */
 	KURL* tempURL = new KURL(destFileDir + strrchr(fileList.at(i), '/'));
	addFileToLists(tempURL->filename());
	tempFileName += fileList.at(i);
	if(++i < fileList.count())
	    tempFileName += '\n';
	}
    //    cout << "Requesting file " << tempFileName << " copied to " << destFileDir << endl;
    connect( getKFM(), SIGNAL( finished() ), this, SLOT( kfmFileCopyComplete() ) );

    /* if there is only one, specify the file name of the dest file, */
    /* else just the dir */
    if(fileList.count() == 1)
	getKFM()->copy(tempFileName, destFileDir + strrchr(fileList.at(0), '/'));
    else
	getKFM()->copy(tempFileName, destFileDir);
    }


void
FileInstallWidget::preHotSync(char* command)
{
  char buffer[10];
  KConfig* config = kapp->getConfig();
  config->setGroup(0L);
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
    }

void 
FileInstallWidget::saveInstallList()
    {
    QString installList = kapp->localkdedir();
    installList += KPILOT_INSTALL_FILE;

    QFile installFile(installList);
    installFile.open(IO_WriteOnly | IO_Truncate);
    QTextStream t(&installFile);

    while(fFileList.first())
	{
	t << fFileList.first()->url() << endl;
	fFileList.remove(fFileList.first());
	}
    installFile.close();
    }


void
FileInstallWidget::refreshFileInstallList()
    {
      unsigned int i;
      fListBox->clear();
      QDir installDir(kapp->localkdedir() + KPILOT_INSTALL_DIR);
      for(i = 2; i < installDir.count(); i++)
	addFileToLists(installDir[i]);
    }
