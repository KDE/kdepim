/***************************************************************************
            filter_evolution_v2.cxx  -  Evolution 2.0.x mail import
                             -------------------
    begin                : Januar 26 2005
    copyright            : (C) 2005 by Danny Kukawka
                           (inspired and partly copied from filter_evolution)
    email                : danny.kukawka@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "filter_evolution_v2.hxx"

#include <config.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <ktempfile.h>


/** Default constructor. */
FilterEvolution_v2::FilterEvolution_v2(void) : 
  Filter(i18n("Import Evolution 2.x Local Mails and Folder Structure"),
	 "Danny Kukawka",
	 i18n("<p><b>Evolution 2.x import filter</b></p>"
              "<p>Select the base directory of your local Evolution mailfolder (usually ~/.evolution/mail/local/).</p>"
              "<p><b>Note:</b> Never choose a Folder, which <u>does not</u> contain mbox-files (for example "
              "a maildir). If you do it anyway, you will get many new folders.</p>"
              "<p>As it is currently impossible to recreate the folder structure, it will be "
	      "\"contained\" in the generated folder's names.</p>"))
{}

/** Destructor. */
FilterEvolution_v2::~FilterEvolution_v2(void) {
   endImport();
}

/** Recursive import of Evolution's mboxes. */
void FilterEvolution_v2::import(FilterInfo *info)
{
  /** 
   * We ask the user to choose Evolution's root directory. 
   * This should be usually ~/.evolution/mail/local/
   */
  QString mailDir = KFileDialog::getExistingDirectory(QDir::homeDirPath(), info->parent());
  info->setOverall(0);

  /** Recursive import of the MailArchives */
  QDir dir(mailDir);
  QStringList rootSubDirs = dir.entryList("[^\\.]*", QDir::Dirs, QDir::Name); // Removal of . and ..
  int currentDir = 1, numSubDirs = rootSubDirs.size();
  for(QStringList::Iterator filename = rootSubDirs.begin() ; filename != rootSubDirs.end() ; ++filename, ++currentDir) {
    importDirContents(info, dir.filePath(*filename), *filename, QString::null);
    info->setOverall((int) ((float) currentDir / numSubDirs * 100));
  }
  
  /** import last but not least all archives from the root-dir */
  QDir importDir (mailDir);
  QStringList files = importDir.entryList("[^\\.]*", QDir::Files, QDir::Name);
  for ( QStringList::Iterator mailFile = files.begin(); mailFile != files.end(); ++mailFile) {
    QString temp_mailfile = *mailFile;
    if (temp_mailfile.endsWith(".cmeta") || temp_mailfile.endsWith(".ev-summary") || 
        temp_mailfile.endsWith(".ibex.index") || temp_mailfile.endsWith(".ibex.index.data") ) {}
    else {  
        info->addLog( i18n("Start import file %1...").arg( temp_mailfile ) );
        importMBox(info, mailDir + "/" + temp_mailfile , temp_mailfile, QString::null);
    }    
  }
  
  info->addLog( i18n("Finished importing emails from %1").arg( mailDir ));
  info->setCurrent(100);
  if(count_duplicates > 0) {
     info->addLog( i18n("1 duplicate message not imported", "%n duplicate messages not imported", count_duplicates));
  }
}

/**
 * Import of a directory contents.
 * @param info Information storage for the operation.
 * @param dirName The name of the directory to import.
 * @param KMailRootDir The directory's root directory in KMail's folder structure.
 * @param KMailSubDir The directory's direct ancestor in KMail's folder structure.
 */
void FilterEvolution_v2::importDirContents(FilterInfo *info, const QString& dirName, const QString& KMailRootDir, const QString& KMailSubDir)
{
    /** Here Import all archives in the current dir */
    QDir dir(dirName);
  
    QDir importDir (dirName);
    QStringList files = importDir.entryList("[^\\.]*", QDir::Files, QDir::Name);
    for ( QStringList::Iterator mailFile = files.begin(); mailFile != files.end(); ++mailFile) {
      QString temp_mailfile = *mailFile;
      if (temp_mailfile.endsWith(".cmeta") || temp_mailfile.endsWith(".ev-summary") || 
          temp_mailfile.endsWith(".ibex.index") || temp_mailfile.endsWith(".ibex.index.data") ) {}
      else {  
          info->addLog( i18n("Start import file %1...").arg( temp_mailfile ) );
          importMBox(info, (dirName + "/" + temp_mailfile) , KMailRootDir, KMailSubDir);
      }    
    }

    /** If there are subfolders, we import them one by one */
    QDir subfolders(dirName);
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

/**
 * Import of a MBox file.
 * @param info Information storage for the operation.
 * @param dirName The MBox's name.
 * @param KMailRootDir The directory's root directory in KMail's folder structure.
 * @param KMailSubDir The directory's equivalent in KMail's folder structure. *
 */
void FilterEvolution_v2::importMBox(FilterInfo *info, const QString& mboxName, const QString& rootDir, const QString& targetDir)
{
  QFile mbox(mboxName);
  if (!mbox.open(IO_ReadOnly)) {
    info->alert(i18n("Unable to open %1, skipping").arg(mboxName));
  } else {
    QFileInfo filenameInfo(mboxName);
    
    info->setCurrent(0);
    info->setFrom(mboxName);
    info->setTo(targetDir);
    
    while (!mbox.atEnd()) {
      KTempFile tmp;
      /** @todo check if the file is really a mbox, maybe search for 'from' string at start */
      /* comment by Danny:
      * Don't use QTextStream to read from mbox, etter use QDataStream. QTextStream only 
      * support Unicode/Latin1/Locale. So you lost information from emails with 
      * charset!=Unicode/Latin1/Locale (e.g. KOI8-R) and Content-Transfer-Encoding != base64 
      * (e.g. 8Bit). It also not help to convert the QTextStream to Unicode. By this you
      * get Unicode/UTF-email but KMail can't detect the correct charset.
      */
      QByteArray input(MAX_LINE);
      QCString seperate;
      mbox.readLine(input.data(),MAX_LINE);
	
      long l = mbox.readLine( input.data(),MAX_LINE); // read the first line, prevent "From "
      tmp.file()->writeBlock( input, l );
	
      while ( ! mbox.atEnd() &&  (l = mbox.readLine(input.data(),MAX_LINE)) && ((seperate = input.data()).left(5) != "From ")) {
	tmp.file()->writeBlock( input, l );
      }
      tmp.close();

      QString destFolder = rootDir;
      QString _targetDir = targetDir;
      if(destFolder.contains(".sbd")) destFolder.remove(".sbd");
      if(!targetDir.isNull()){
         if(_targetDir.contains(".sbd")) _targetDir.remove(".sbd");
         destFolder += ("-" + _targetDir);
         destFolder += "-" + filenameInfo.baseName(TRUE);// mboxName;
      }
      
      if(info->removeDupMsg) addMessage( info, destFolder, tmp.name() );
      else addMessage_fastImport( info, destFolder, tmp.name() );
      
      tmp.unlink();
      int currentPercentage = (int) (((float) mbox.at() / filenameInfo.size()) * 100);
      info->setCurrent(currentPercentage);
      if (info->shouldTerminate()) return;
    }
    mbox.close();
  }
  
}

