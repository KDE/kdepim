/***************************************************************************
                          filter_mbox.cxx  -  mbox mail import
                             -------------------
    begin                : Sat Apr 5 2003
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

#include "filter_mbox.hxx"

FilterMBox::FilterMBox() :
Filter(	i18n("Import mbox archives"),
    "Laurence Anderson",
    i18n("<p><b>mbox import filter</b></p>"
      "<p>This filter will import mbox files into kmail</p>"
      "<p><b>Note:</b> Emails will be imported into folders named after the file they came from, prefixed with MBOX-</p>"
      ))
{
}

FilterMBox::~FilterMBox()
{
}

void FilterMBox::import(FilterInfo *info)
{
  int currentFile = 0;
  QStringList filenames = KFileDialog::getOpenFileNames( QDir::homeDirPath(), "*", info->parent() );
  info->setOverall(0);

  for ( QStringList::Iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
    QFile mbox( *filename );
    if (! mbox.open( IO_ReadOnly ) ) {
      info->alert( i18n("Couldn't open %1, skipping").arg( *filename ) );
    }

    QFileInfo filenameInfo( *filename );
    QString folderName( "MBOX-" + filenameInfo.baseName(TRUE) );
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
      addMessage( info, folderName, tmp.name() );
      tmp.unlink();
      info->setCurrent( (int) (((float) mbox.at() / filenameInfo.size()) * 100) );
    }
  
    info->setCurrent(100);
    info->setOverall( (int) ( (float) ++currentFile / filenames.count() ) );
    info->addLog( i18n("Finished importing emails from %1").arg( *filename ) );
  }
  info->setOverall(100);
}

// vim: ts=2 sw=2 et
