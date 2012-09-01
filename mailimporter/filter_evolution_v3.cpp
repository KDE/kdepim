/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filter_evolution_v3.h"

#include <klocale.h>
#include <kfiledialog.h>

using namespace MailImporter;

/** Default constructor. */
FilterEvolution_v3::FilterEvolution_v3()
  :Filter(i18n("Import Evolution 3.x Local Mails and Folder Structure"),
          "Laurent Montel",
          i18n("<p><b>Evolution 3.x import filter</b></p>"
               "<p>Select the base directory of your local Evolution mailfolder (usually ~/.local/share/evolution/mail/local/).</p>"
               "<p>Since it is possible to recreate the folder structure, the folders "
               "will be stored under: \"Evolution-Import\".</p>") )
{
}

                

/** Destructor. */
FilterEvolution_v3::~FilterEvolution_v3()
{
}

QString FilterEvolution_v3::defaultSettingsPath()
{
  return QDir::homePath() + QLatin1String( "/.local/share/evolution/mail/local/" );
}


/** Recursive import of KMail maildir. */
void FilterEvolution_v3::import()
{
  setCountDuplicates(0);
  QString evolDir = defaultSettingsPath();
  QDir d( evolDir );
  if ( !d.exists() ) {
    evolDir = QDir::homePath();
  }
    
  KFileDialog *kfd = new KFileDialog( evolDir, "", 0 );
  kfd->setMode( KFile::Directory | KFile::LocalOnly );
  kfd->exec();
  const QString dir = kfd->selectedFile();
  delete kfd;

  importMails( dir );

}

void FilterEvolution_v3::processDirectory( const QString& path)
{
  QDir dir(path);
  const QStringList rootSubDirs = dir.entryList(QStringList("*"), QDir::Dirs | QDir::Hidden, QDir::Name);
  QStringList::ConstIterator end = rootSubDirs.constEnd();
  for(QStringList::ConstIterator filename = rootSubDirs.constBegin() ; filename != end ; ++filename) {
    if(filterInfo()->shouldTerminate())
      break;
    if(!(*filename == QLatin1String( "." ) || *filename == QLatin1String( ".." ))) {
      filterInfo()->setCurrent(0);
      importDirContents(dir.filePath(*filename));
      filterInfo()->setOverall((int) ((float) mImportDirDone / mTotalDir * 100));
      filterInfo()->setCurrent(100);
    }
  }
  
}
  
void FilterEvolution_v3::importMails( const QString& maildir )
{
  setMailDir(maildir);
  if ( mailDir().isEmpty() ) {
    filterInfo()->alert( i18n( "No directory selected." ) );
    return;
  }
  /**
   * If the user only select homedir no import needed because
   * there should be no files and we surely import wrong files.
   */
  else if ( mailDir() == QDir::homePath() || mailDir() == ( QDir::homePath() + '/' ) ) {
    filterInfo()->addErrorLogEntry( i18n( "No files found for import." ) );
  } else {
    filterInfo()->setOverall(0);
    mImportDirDone = 0;

    /** Recursive import of the MailArchives */
    QDir dir(mailDir());
    mTotalDir = Filter::countDirectory( dir, true /*search hidden directory*/ );

    processDirectory( mailDir() );
    
    filterInfo()->addInfoLogEntry( i18n("Finished importing emails from %1", mailDir() ));

    if (countDuplicates() > 0) {
        filterInfo()->addInfoLogEntry( i18np("1 duplicate message not imported", "%1 duplicate messages not imported", countDuplicates()));
    }

    if (filterInfo()->shouldTerminate())
        filterInfo()->addInfoLogEntry( i18n("Finished import, canceled by user."));
  }
  filterInfo()->setCurrent(100);
  filterInfo()->setOverall(100);
}

/**
 * Import of a directory contents.
 * @param info Information storage for the operation.
 * @param dirName The name of the directory to import.
 */
void FilterEvolution_v3::importDirContents( const QString& dirName)
{

  /** Here Import all archives in the current dir */
  importFiles(dirName);

  /** If there are subfolders, we import them one by one */
  processDirectory( dirName );
}

/**
 * Import the files within a Folder.
 * @param info Information storage for the operation.
 * @param dirName The name of the directory to import.
 */
void FilterEvolution_v3::importFiles( const QString& dirName)
{

  QDir dir(dirName);
  QString _path;
  bool generatedPath = false;

  QDir importDir (dirName);
  const QStringList files = importDir.entryList(QStringList("[^\\.]*"), QDir::Files, QDir::Name);
  int currentFile = 1, numFiles = files.size();
  QStringList::ConstIterator filesEnd( files.constEnd() );
    
  for ( QStringList::ConstIterator mailFile = files.constBegin(); mailFile != filesEnd; ++mailFile, ++currentFile) {
    if(filterInfo()->shouldTerminate()) return;
    QString temp_mailfile = *mailFile;
    if (!( temp_mailfile.endsWith(QLatin1String(".db"))
           || temp_mailfile.endsWith(QLatin1String(".cmeta"))
           || temp_mailfile.endsWith(QLatin1String(".ev-summary"))
           || temp_mailfile.endsWith(QLatin1String(".ibex.index"))
           || temp_mailfile.endsWith(QLatin1String(".ibex.index.data")) ) ) {
      if(!generatedPath) {
        _path = i18nc("define folder name where we import evolution mails", "Evolution-Import" );
        QString _tmp = dir.filePath(*mailFile);
        _tmp = _tmp.remove( mailDir(), Qt::CaseSensitive );
        QStringList subFList = _tmp.split( QLatin1Char( '/' ), QString::SkipEmptyParts );
        QStringList::ConstIterator end( subFList.end() ); 
        for ( QStringList::ConstIterator it = subFList.constBegin(); it != end; ++it ) {
          QString _cat = *it;
          if(!(_cat == *mailFile)) {
            if (_cat.startsWith(QLatin1Char( '.' ))) {
              _cat = _cat.remove(0 , 1);
            }
            //Evolution store inbox as "."
            if ( _cat.startsWith(QLatin1Char( '.' ))) {
              _cat = _cat.replace( 0, 1, QLatin1String( "Inbox/" ) );
            }
              
            _path += QLatin1Char( '/' ) + _cat;
            _path.replace( QLatin1Char( '.' ), QLatin1Char( '/' ) );
          }
        }
        if(_path.endsWith(QLatin1String( "cur" )))
          _path.remove(_path.length() - 4 , 4);
        QString _info = _path;
        filterInfo()->addInfoLogEntry(i18n("Import folder %1...", _info));
        filterInfo()->setFrom(_info);
        filterInfo()->setTo(_path);
        generatedPath = true;
      }
      Akonadi::MessageStatus status = statusFromFile( *mailFile );
      
      if(filterInfo()->removeDupMessage()) {
        if(! addMessage( _path, dir.filePath(*mailFile),status )) {
          filterInfo()->addErrorLogEntry( i18n("Could not import %1", *mailFile ) );
        }
        filterInfo()->setCurrent((int) ((float) currentFile / numFiles * 100));
      } else {
        if(! addMessage_fastImport( _path, dir.filePath(*mailFile),status )) {
          filterInfo()->addErrorLogEntry( i18n("Could not import %1", *mailFile ) );
        }
        filterInfo()->setCurrent((int) ((float) currentFile / numFiles * 100));
      }
    }
  }
}

Akonadi::MessageStatus FilterEvolution_v3::statusFromFile( const QString& filename)
{
  Akonadi::MessageStatus status;
  const int statusIndex = filename.indexOf( ":2," );
  if ( statusIndex != -1 ) {
    const QString statusStr = filename.right( filename.length() - statusIndex -3 );
    if ( statusStr.contains( QLatin1Char( 'S' ) ) ) {
      status.setRead( true );
    }
    if ( statusStr.contains( QLatin1Char( 'F' ) ) ) {
      
    }
    if ( statusStr.contains( QLatin1Char( 'R' ) ) ) {
      status.setReplied( true );
    }
    if ( statusStr.contains( QLatin1Char( 'P' ) ) ) {
      status.setForwarded( true );
    }
  }
  return status;
}
  
