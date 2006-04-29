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
        Filter( i18n("Import mbox Files (UNIX, Evolution)"),
                "Laurence Anderson <p>( Filter accelerated by Danny Kukawka )</p>",
                i18n("<p><b>mbox import filter</b></p>"
                     "<p>This filter will import mbox files into KMail. Use this filter "
                     "if you want to import mails from Ximian Evolution or other mailers "
                     "that use this traditional UNIX format.</p>"
                     "<p><b>Note:</b> Emails will be imported into folders named after the "
                     "file they came from, prefixed with MBOX-</p>" ))
{}

FilterMBox::~FilterMBox()
{
    endImport();
}

void FilterMBox::import(FilterInfo *info)
{
    int currentFile = 1;
    int overall_status = 0;
    bool first_msg = true;

    QStringList filenames = KFileDialog::getOpenFileNames( QDir::homePath(), "*|" + i18n("mbox Files (*)"), info->parent() );
    info->setOverall(0);

    for ( QStringList::Iterator filename = filenames.begin(); filename != filenames.end(); ++filename, ++currentFile) {
        QFile mbox( *filename );
        if (! mbox.open( QIODevice::ReadOnly ) ) {
            info->alert( i18n("Unable to open %1, skipping", *filename ) );
        } else {
            QFileInfo filenameInfo( *filename );
            QString folderName( "MBOX-" + filenameInfo.completeBaseName() );

            info->setCurrent(0);
            info->addLog( i18n("Importing emails from %1...", *filename ) );

            info->setFrom( *filename );
            info->setTo( folderName );

            QByteArray input(MAX_LINE,'\0');
            long l = 0;

            while ( ! mbox.atEnd() ) {
                KTempFile tmp;
                QIODevice::Offset filepos = 0;
                /* comment by Danny:
                * Don't use QTextStream to read from mbox, better use QDataStream. QTextStream only 
                * support Unicode/Latin1/Locale. So you lost information from emails with 
                * charset!=Unicode/Latin1/Locale (e.g. KOI8-R) and Content-Transfer-Encoding != base64 
                * (e.g. 8Bit). It also not help to convert the QTextStream to Unicode. By this you
                * get Unicode/UTF-email but KMail can't detect the correct charset.
                */
                QByteArray seperate;

                if(!first_msg)
                    tmp.file()->write( input, l );
                l = mbox.readLine( input.data(),MAX_LINE); // read the first line, prevent "From "
                tmp.file()->write( input, l );

                while ( ! mbox.atEnd() &&  (l = mbox.readLine(input.data(),MAX_LINE)) && ((seperate = input.data()).left(5) != "From ")) {
                       tmp.file()->write( input, l );
                       
                       // workaround to fix hang if a corrupted mbox contains some 
                       // binary data, for more see bug #106796
                       if (mbox.pos() == filepos)
                           mbox.seek(mbox.size());
                       else 
                           filepos = mbox.pos();
                }
                tmp.close();
                first_msg = false;

                /* comment by Danny Kukawka:
                * addMessage() == old function, need more time and check for duplicates
                * addMessage_fastImport == new function, faster and no check for duplicates
                */
                if(info->removeDupMsg)
                    addMessage( info, folderName, tmp.name() );
                else
                    addMessage_fastImport( info, folderName, tmp.name() );

                tmp.unlink();
                int currentPercentage = (int) ( ( (float) mbox.pos() / filenameInfo.size() ) * 100 );
                info->setCurrent( currentPercentage );
                if (currentFile == 1)
                    overall_status = (int)( currentPercentage*((float)currentFile/filenames.count()));
                else
                    overall_status = (int)(((currentFile-1)*(100.0/(float)filenames.count()))+(currentPercentage*(1.0/(float)filenames.count())));
                info->setOverall( overall_status );

                if ( info->shouldTerminate() ) break;
            }

            info->addLog( i18n("Finished importing emails from %1", *filename ));
            if (count_duplicates > 0) {
                info->addLog( i18np("1 duplicate message not imported to folder %1 in KMail", 
                                   "%n duplicate messages not imported to folder %1 in KMail", 
                                   count_duplicates, folderName));
            }
            if (info->shouldTerminate()) info->addLog( i18n("Finished import, canceled by user."));
            count_duplicates = 0;
            // don't forget to close the file !!!
            mbox.close();
        }
    }
}
