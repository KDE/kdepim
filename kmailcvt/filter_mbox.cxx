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
Filter(	i18n("Import mbox Files (UNIX, Evolution)"),
    "Laurence Anderson <p>( Filter accelerated by Danny Kukawka )</p>",
    i18n("<p><b>mbox import filter</b></p>"
      "<p>This filter will import mbox files into KMail. Use this filter "
      "if you want to import mails from Ximian Evolution or other mailers that use this traditional UNIX format.</p>"
      "<p><b>Note:</b> Emails will be imported into folders named after the file they came from, prefixed with MBOX-</p>"
      ))
{
}

FilterMBox::~FilterMBox()
{ 
   endImport();
}

void FilterMBox::import(FilterInfo *info)
{
  int currentFile = 1;
  int overall_status = 0;
  
  QStringList filenames = KFileDialog::getOpenFileNames( QDir::homeDirPath(), "*|" + i18n("mbox Files (*)"), info->parent() );
  info->setOverall(0);
  
  for ( QStringList::Iterator filename = filenames.begin(); filename != filenames.end(); ++filename, ++currentFile) {
    QFile mbox( *filename );
    if (! mbox.open( IO_ReadOnly ) ) {
      info->alert( i18n("Unable to open %1, skipping").arg( *filename ) );
    }
    else { 
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
          overall_status = (int)( currentPercentage*((float)currentFile/filenames.count()));
        else
          overall_status = (int)(((currentFile-1)*(100.0/(float)filenames.count()))+(currentPercentage*(1.0/(float)filenames.count())));
	info->setOverall( overall_status );
	
	if ( info->shouldTerminate() ) return;
      }
      
      info->addLog( i18n("Finished importing emails from %1").arg( *filename ));
      if (count_duplicates > 0) {
	  info->addLog( i18n("1 duplicate message not imported to folder %1 in KMail", "%n duplicate messages not imported to folder %1 in KMail", count_duplicates).arg(folderName));
      }
      count_duplicates = 0;
      // don't forget to close the file !!!
      mbox.close();
    }
  }
}

// vim: ts=2 sw=2 et
