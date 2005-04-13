/***************************************************************************
                  filter_opera.cxx  -  Opera mail import
                             -------------------
    begin                : Januar 26 2005
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

#include <config.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <ktempfile.h>
#include <kdebug.h>

#include "filter_opera.hxx"


FilterOpera::FilterOpera() :
        Filter( i18n("Import Opera Emails"),
                "Danny Kukawka",
                i18n("<p><b>Opera email import filter</b></p>"
                     "<p>This filter will import mails from Opera mail folder. Use this filter "
                     "if you want to import all mails within a account in the Opera maildir.</p>"
                     "<p>Select the directory of the account (usually ~/.opera/mail/store/account*).</p>"
                     "<p><b>Note:</b> Emails will be imported into a folder named after the account "
                     "they came from, prefixed with OPERA-</p>" ))
{}

FilterOpera::~FilterOpera()
{
    endImport();
}

void FilterOpera::import(FilterInfo *info)
{
    /** try to go to opera mailfolder in the home of the user */
    QString startdir = QDir::homeDirPath() + "/.opera/mail/store/";
    QDir d( startdir );
    if ( !d.exists() ) {
        startdir = QDir::homeDirPath();
    }

    //QString mailDir = KFileDialog::getExistingDirectory(QDir::homeDirPath(), info->parent());
    KFileDialog *kfd;
    kfd = new KFileDialog( startdir, "", 0, "kfiledialog", true );
    kfd->setMode(KFile::Directory | KFile::LocalOnly);
    kfd->exec();
    QString operaDir = kfd->selectedFile();

    if (operaDir.isEmpty()) {
        info->alert(i18n("No directory selected."));
    }
    /**
     * If the user only select homedir no import needed because 
     * there should be no files and we surely import wrong files.
     */
    else if ( operaDir == QDir::homeDirPath() || operaDir == (QDir::homeDirPath() + "/")) {
        info->addLog(i18n("No files found for import."));
    } else {
        info->setOverall(0);

        QDir importDir (operaDir);
        QStringList files = importDir.entryList("*.[mM][bB][sS]", QDir::Files, QDir::Name);

        // Count total number of files to be processed
        info->addLog(i18n("Counting files..."));
        int totalFiles = files.count();
        int currentFile = 1;

        if(totalFiles > 0) {
            int overall_status = 0;

            info->addLog(i18n("Importing new mail files..."));
            for ( QStringList::Iterator mailFile = files.begin(); mailFile != files.end(); ++mailFile) {
                info->setCurrent(0);
                QFile operaArchiv( importDir.filePath(*mailFile) );
                if (! operaArchiv.open( IO_ReadOnly ) ) {
                    info->alert( i18n("Unable to open %1, skipping").arg( *mailFile ) );
                } else {
                    info->addLog( i18n("Importing emails from %1...").arg( *mailFile ) );
                    QFileInfo filenameInfo( importDir.filePath(*mailFile) );
                    QString folderName( "OPERA-" + importDir.dirName() );

                    info->setFrom( *mailFile );
                    info->setTo( folderName );

                    QByteArray input(MAX_LINE);
                    long l = 0;
                    bool first_msg = true;

                    while ( !operaArchiv.atEnd() ) {
                        KTempFile tmp;
                        /* comment by Danny:
                        * Don't use QTextStream to read from mbox, etter use QDataStream. QTextStream only 
                        * support Unicode/Latin1/Locale. So you lost information from emails with 
                        * charset!=Unicode/Latin1/Locale (e.g. KOI8-R) and Content-Transfer-Encoding != base64 
                        * (e.g. 8Bit). It also not help to convert the QTextStream to Unicode. By this you
                        * get Unicode/UTF-email but KMail can't detect the correct charset.
                        */
                        QCString seperate;

                        if(!first_msg)
                            tmp.file()->writeBlock( input, l );
                        l = operaArchiv.readLine( input.data(),MAX_LINE); // read the first line, prevent "From "
                        tmp.file()->writeBlock( input, l );

                        while ( ! operaArchiv.atEnd() &&  (l = operaArchiv.readLine(input.data(),MAX_LINE)) && ((seperate = input.data()).left(5) != "From ")) {
                            /** remove in KMail unneeded Flags from Opera (for example: X-Opera-Status)*/
                            if(seperate.left(8) != "X-Opera-")
                                tmp.file()->writeBlock( input, l );
                        }
                        tmp.close();
                        first_msg = false;

                        if(info->removeDupMsg)
                            addMessage( info, folderName, tmp.name() );
                        else
                            addMessage_fastImport( info, folderName, tmp.name() );
                        tmp.unlink();
                        int currentPercentage = (int) ( ( (float) operaArchiv.at() / filenameInfo.size() ) * 100 );
                        info->setCurrent( currentPercentage );

                        if (currentFile == 1)
                            overall_status = (int) ( currentPercentage * ( (float) currentFile / totalFiles ) );
                        else
                            overall_status = (int)(((currentFile-1)*(100.0/(float)totalFiles))+(currentPercentage*(1.0/(float)totalFiles)));

                        info->setOverall( overall_status );
                        if ( info->shouldTerminate() ) break;
                    }

                    info->addLog( i18n("Finished importing emails from %1").arg( *mailFile ));
                    if (count_duplicates > 0) {
                        info->addLog( i18n("1 duplicate message not imported", "%n duplicate messages not imported", count_duplicates));
                    }
                    currentFile++;
                    count_duplicates = 0;
                    operaArchiv.close();
                }
                if ( info->shouldTerminate() ) break;
            }
        } else {
            info->addLog(i18n("No files found for import."));
        }
    }
    if (info->shouldTerminate()) info->addLog( i18n("Finished import, canceled by user."));
    info->setCurrent(100);
    info->setOverall(100);
}
