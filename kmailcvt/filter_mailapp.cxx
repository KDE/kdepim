/***************************************************************************
                          filter_mailapp.cxx  -  OS X Mail App import
                             -------------------
    copyright            : (C) 2004 by Chris Howells
    email                : howells@kde.org
 
    Derived from code by:
    copyright            : (C) 2003 by Laurence Anderson
    email                : l.d.anderson@warwick.ac.uk

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <config.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <ktempfile.h>
#include <kdebug.h>

#include "filter_mailapp.hxx"

FilterMailApp::FilterMailApp() :
Filter(	i18n("Import From OS X Mail"),
    "Chris Howells<br /><br />Filter accelerated by Danny Kukawka )",
    i18n("<p><b>OS X Mail Import Filter</b></p>"
      "<p>This filter imports e-mails from the Mail client in Apple Mac OS X.</p>"
      ))
{
}

FilterMailApp::~FilterMailApp()
{
  endImport();
}

void FilterMailApp::import(FilterInfo *info)
{
  int currentFile = 1;
  int overall_status = 0;
  
  QString directory = KFileDialog::getExistingDirectory( QDir::homeDirPath(), info->parent() );
  info->setOverall(0);

//   kdDebug() << "starting by looking in directory " << directory << endl;
  traverseDirectory(directory);
  
  for ( QStringList::Iterator filename = mMboxFiles.begin(); filename != mMboxFiles.end(); ++filename, ++currentFile) {
    QFile mbox( *filename );
    if (! mbox.open( IO_ReadOnly ) ) {
      info->alert( i18n("Unable to open %1, skipping").arg( *filename ) );
    }
    else {
      QFileInfo filenameInfo( *filename );
      kdDebug() << "importing filename " << *filename << endl;
      QStringList name = QStringList::split("/", *filename);
      QString folderName(name[name.count() - 2]);
      QTextStream mboxFile( &mbox );
      QString mboxLine = mboxFile.readLine();
      
      info->setCurrent(0);
      info->addLog( i18n("Importing emails from %1...").arg( *filename ) );
      info->setFrom( *filename );
      info->setTo( folderName );
      
      while ( ! mboxFile.atEnd() ) {
      KTempFile tmp;
      *tmp.textStream() << mboxLine << endl; // Not really needed (the From line)
      while ( !( mboxLine = mboxFile.readLine() ).isNull() && mboxLine.left(5) != "From " )
              *tmp.textStream() << mboxLine << endl;
      tmp.close();
      
      /* comment by Danny Kukawka:
       * addMessage() == old function, need more time and check for duplicates
       * addMessage_fastImport == new function, faster and no check for duplicates
       */
      if(info->removeDupMsg) addMessage( info, folderName, tmp.name() );
      else addMessage_fastImport( info, folderName, tmp.name() );
      
      tmp.unlink();
      
      int currentPercentage = (int) ( ( (float) mbox.at() / filenameInfo.size() ) * 100 );
      info->setCurrent( currentPercentage );
      if (currentFile == 1)
        overall_status = (int)( currentPercentage*((float)currentFile/mMboxFiles.count()));
      else
        overall_status = (int)(((currentFile-1)*(100.0/(float)mMboxFiles.count()))+(currentPercentage*(1.0/(float)mMboxFiles.count())));
      info->setOverall( overall_status );
      if ( info->shouldTerminate() ) return;
      }
      
      info->addLog( i18n("Finished importing emails from %1").arg( *filename ) );
      if (count_duplicates > 0) {
	info->addLog( i18n("1 duplicate message not imported to folder %1 in KMail", "%n duplicate messages not imported to folder %1 in KMail", count_duplicates).arg(folderName));
      }
      count_duplicates = 0;
      mbox.close();
    }
  }
}

void FilterMailApp::traverseDirectory(const QString &dirName)
{
  QDir dir(dirName);
  dir.setFilter(QDir::Dirs | QDir::Files);

  const QFileInfoList *fileinfolist = dir.entryInfoList();
  QFileInfoListIterator it(*fileinfolist);
  QFileInfo *fi;
  while ((fi = it.current())) {
     if (fi->fileName() == "." || fi->fileName() == "..") {
        ++it;
        continue;
     }
     if (fi->isDir() && fi->isReadable()) {
        traverseDirectory(fi->filePath());
     }
     else {
        if (!fi->isDir() && fi->fileName() == "mbox") {
	   kdDebug() << "adding the file " << fi->filePath() << endl;
           mMboxFiles.append(fi->filePath());
        }
     }
  ++it;
 }
}

// vim: ts=2 sw=2 et
