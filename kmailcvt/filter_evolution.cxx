/*
 *  filter_evolution.cxx
 *  Author : Simon MARTIN <simartin@users.sourceforge.net>
 *  Copyright (c) 2004 Simon MARTIN
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "filter_evolution.hxx"

#include <config.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <ktempfile.h>


/** Default constructor. */
FilterEvolution::FilterEvolution(void) : 
  Filter(i18n("Import Evolution Mails and Folder Structure"),
	 "Simon MARTIN<br /><br />( Filter accelerated by Danny Kukawka )",
	 i18n("<p>Select the base directory of Evolution's mails (usually ~/evolution/local).</p>"
              "<p>As it is currently impossible to recreate the folder structure, it will be "
	      "\"contained\" in the generated folder's names.</p>"
	      "<p>For instance, if you have a \"foo\" folder in Evolution, with a \"bar\" subfolder, "
	      "two folders will be created in KMail : \"foo\" and \"foo-bar\"</p>"))
{}

/** Destructor. */
FilterEvolution::~FilterEvolution(void) {
   endImport();
}

/** Recursive import of Evolution's mboxes. */
void FilterEvolution::import(FilterInfo *info)
{
  // We ask the user to choose Evolution's root directory.
  QString mailDir = KFileDialog::getExistingDirectory(QDir::homeDirPath(), info->parent());
  info->setOverall(0);

  // Recursive import of the MBoxes.
  QDir dir(mailDir);
  QStringList rootSubDirs = dir.entryList("[^\\.]*", QDir::Dirs, QDir::Name); // Removal of . and ..
  int currentDir = 1, numSubDirs = rootSubDirs.size();
  for(QStringList::Iterator filename = rootSubDirs.begin() ; filename != rootSubDirs.end() ; ++filename, ++currentDir) {
    importDirContents(info, dir.filePath(*filename), *filename, QString::null);
    info->setOverall((int) ((float) currentDir / numSubDirs * 100));
  }
}

/**
 * Import of a directory contents.
 * @param info Information storage for the operation.
 * @param dirName The name of the directory to import.
 * @param KMailRootDir The directory's root directory in KMail's folder structure.
 * @param KMailSubDir The directory's direct ancestor in KMail's folder structure.
 */
void FilterEvolution::importDirContents(FilterInfo *info, const QString& dirName, const QString& KMailRootDir, const QString& KMailSubDir)
{
  // If there is a mbox, we import it
  QDir dir(dirName);
  if(dir.exists("mbox")) {
    importMBox(info, dirName + "/mbox", KMailRootDir, KMailSubDir);
  }
  // If there are subfolders, we import them one by one
  if(dir.exists("subfolders")) {
    QDir subfolders(dirName + "/subfolders");
    QStringList subDirs = subfolders.entryList("[^\\.]*", QDir::Dirs, QDir::Name);
    for(QStringList::Iterator filename = subDirs.begin() ; filename != subDirs.end() ; ++filename) {
      QString kSubDir;
      if(!KMailSubDir.isNull()) {
	kSubDir = KMailSubDir + "-" + *filename;
      } else {
	kSubDir = *filename;
      }
      importDirContents(info, subfolders.filePath(*filename), KMailRootDir, kSubDir);
    }
  }
}

/**
 * Import of a MBox file.
 * @param info Information storage for the operation.
 * @param dirName The MBox's name.
 * @param KMailRootDir The directory's root directory in KMail's folder structure.
 * @param KMailSubDir The directory's equivalent in KMail's folder structure. *
 */
void FilterEvolution::importMBox(FilterInfo *info, const QString& mboxName, const QString& rootDir, const QString& targetDir)
{
  QFile mbox(mboxName);
  if (!mbox.open(IO_ReadOnly)) {
    info->alert(i18n("Unable to open %1, skipping").arg(mboxName));
  } else {
    QFileInfo filenameInfo(mboxName);
    QTextStream mboxFile(&mbox);
    QString mboxLine = mboxFile.readLine();
    
    info->setCurrent(0);
    info->addLog(i18n("Importing emails from %1...").arg(mboxName));
    info->setFrom(mboxName);
    info->setTo(targetDir);
    
    while (!mboxFile.atEnd()) {
      KTempFile tmp;
      *tmp.textStream() << mboxLine << endl; // Not really needed (the From line)
      while (!(mboxLine = mboxFile.readLine()).isNull() && mboxLine.left(5) != "From ")
        *tmp.textStream() << mboxLine << endl;
      tmp.close();
      QString destFolder = rootDir;
      if(!targetDir.isNull())
	destFolder = destFolder + "-" + targetDir;
      
      /* comment by Danny Kukawka:
       * addMessage() == old function, need more time and check for duplicates
       * addMessage_fastImport == new function, faster and no check for duplicates
       */
      if(info->removeDupMsg) addMessage( info, destFolder, tmp.name() );
      else addMessage_fastImport( info, destFolder, tmp.name() );
      
      tmp.unlink();
      int currentPercentage = (int) (((float) mbox.at() / filenameInfo.size()) * 100);
      info->setCurrent(currentPercentage);
      if (info->shouldTerminate()) return;
    }
  
    info->addLog(i18n("Finished importing emails from %1").arg(mboxName));
    if (count_duplicates > 0)
    {
	info->addLog( i18n("1 duplicate message not imported", "%n duplicate messages not imported", count_duplicates));
    }
    count_duplicates = 0;
    mbox.close();
  }
}
