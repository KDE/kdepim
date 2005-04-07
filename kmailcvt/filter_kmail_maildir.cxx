/***************************************************************************
           filter_kmail_maildir.cxx  -  Kmail maildir mail import
                            -------------------
   begin                : April 06 2005
   copyright            : (C) 2005 by Danny Kukawka
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

#include "filter_kmail_maildir.hxx"

#include <config.h>
#include <klocale.h>
#include <kfiledialog.h>

/** Default constructor. */
FilterKMail_maildir::FilterKMail_maildir( void ) : 
		Filter( i18n( "Import KMail Maildirs and Folder Structure" ),
		        "Danny Kukawka",
		        i18n( "<p><b>KMail import filter</b></p>"
		              "<p>Select the base directory of your KMail mailfolder which you would import.</p>"
		              "<p><b>Note:</b> Never select your current local KMail maildir (usually "
		              "~/Mail or ~/.kde/share/apps/kmail/mail ). In this case KMailCVT maybe come into "
		              "a continuous loop. </p>"
		              "<p>This filter doesn't import KMail mailfolder with mbox files.</p>"
		              "<p>Since it is possible to recreate the folder structure all folder "
		              "stored under: \"KMail-Import\" in your local folder.</p>" ) ) {}

/** Destructor. */
FilterKMail_maildir::~FilterKMail_maildir( void ) {
	endImport();
}

/** Recursive import of KMail maildir. */
void FilterKMail_maildir::import( FilterInfo *info ) {

	QString _homeDir = QDir::homeDirPath();

	KFileDialog *kfd;
	kfd = new KFileDialog( _homeDir, "", 0, "kfiledialog", true );
	kfd->setMode( KFile::Directory | KFile::LocalOnly );
	kfd->exec();
	mailDir = kfd->selectedFile();

	if ( mailDir.isEmpty() ) {
		info->alert( i18n( "No directory selected." ) );
	}
	/**
	 * If the user only select homedir no import needed because 
	 * there should be no files and we shurely import wrong files.
	 */
	else if ( mailDir == QDir::homeDirPath() || mailDir == ( QDir::homeDirPath() + "/" ) ) {
		info->addLog( i18n( "No files found for import." ) );
	} else {
		info->setOverall(0);

		/** Recursive import of the MailArchives */
		QDir dir(mailDir);
		QStringList rootSubDirs = dir.entryList("*", QDir::Dirs | QDir::Hidden, QDir::Name);
		int currentDir = 1, numSubDirs = rootSubDirs.size();
		for(QStringList::Iterator filename = rootSubDirs.begin() ; filename != rootSubDirs.end() ; ++filename, ++currentDir) {
			if(!(*filename == "." || *filename == "..")) {
				info->setCurrent(0);
				importDirContents(info, dir.filePath(*filename));
				info->setOverall((int) ((float) currentDir / numSubDirs * 100));
				info->setCurrent(100);
			}
		}
	}
	
	info->addLog( i18n("Finished importing emails from %1").arg( mailDir ));
	if (count_duplicates > 0) {
		info->addLog( i18n("1 duplicate message not imported", "%n duplicate messages not imported", count_duplicates));
	}
	count_duplicates = 0;
	info->setCurrent(100);
	info->setOverall(100);
}

/**
 * Import of a directory contents.
 * @param info Information storage for the operation.
 * @param dirName The name of the directory to import.
 * @param KMailRootDir The directory's root directory in KMail's folder structure.
 */
void FilterKMail_maildir::importDirContents( FilterInfo *info, const QString& dirName) {

	/** Here Import all archives in the current dir */
	importFiles(info, dirName);
	
	/** If there are subfolders, we import them one by one */

	QDir subfolders(dirName);
	QStringList subDirs = subfolders.entryList("*", QDir::Dirs | QDir::Hidden, QDir::Name);
	for(QStringList::Iterator filename = subDirs.begin() ; filename != subDirs.end() ; ++filename) {
		if(!(*filename == "." || *filename == "..")) {
			importDirContents(info, subfolders.filePath(*filename));
		}
	}
}

/**
 * Import the files within a Folder.
 * @param info Information storage for the operation.
 * @param dirName The name of the directory to import.
 */
void FilterKMail_maildir::importFiles( FilterInfo *info, const QString& dirName) {
	
	QDir dir(dirName);
	QString _path = "";
	bool generatedPath = false;

	QDir importDir (dirName);
	QStringList files = importDir.entryList("[^\\.]*", QDir::Files, QDir::Name);
	int currentFile = 1, numFiles = files.size();
	for ( QStringList::Iterator mailFile = files.begin(); mailFile != files.end(); ++mailFile, ++currentFile) {
		QString temp_mailfile = *mailFile;
		if (!(temp_mailfile.endsWith(".index") || temp_mailfile.endsWith(".index.ids") ||
			temp_mailfile.endsWith(".index.sorted") || temp_mailfile.endsWith(".uidcache") )) {
			if(!generatedPath) {
				_path = "KMail-Import";
				QString _tmp = dir.filePath(*mailFile);
				_tmp = _tmp.remove( mailDir ,TRUE);
				QStringList subFList = QStringList::split("/",_tmp,FALSE);
				for ( QStringList::Iterator it = subFList.begin(); it != subFList.end(); ++it ) {
					QString _cat = *it;
					if(!(_cat == *mailFile)) {
						if(_cat.startsWith(".") && _cat.endsWith(".directory")) {
							_cat.remove(0,1);
							_cat.remove((_cat.length() - 10), 10);
						}
						else if (_cat.startsWith(".")) {
							_cat = _cat.remove(0 , 1);
						}
						_path += "/" + _cat;
					}
				}
				if(_path.endsWith("cur")) _path.remove(_path.length() - 4 , 4);
				QString _info = _path;
				info->addLog(i18n("Import folder %1...").arg(_info.remove(0,12)));
				info->setFrom(_info);
				info->setTo(_path);
				generatedPath = true;
			}
			
			if(info->removeDupMsg) {
				if(! addMessage( info, _path, dir.filePath(*mailFile) )) {
					info->addLog( i18n("Could not import %1").arg( *mailFile ) );
				}
				info->setCurrent((int) ((float) currentFile / numFiles * 100));
			} else {
				if(! addMessage_fastImport( info, _path, dir.filePath(*mailFile) )) {
					info->addLog( i18n("Could not import %1").arg( *mailFile ) );
				}
				info->setCurrent((int) ((float) currentFile / numFiles * 100));
			}
		}
	}
}
