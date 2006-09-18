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
        Filter( i18n("Import From OS X Mail"),
                "Chris Howells<br /><br />Filter accelerated by Danny Kukawka )",
                i18n("<p><b>OS X Mail Import Filter</b></p>"
                     "<p>This filter imports e-mails from the Mail client in Apple Mac OS X.</p>"))
{}

FilterMailApp::~FilterMailApp()
{
    endImport();
}

void FilterMailApp::import(FilterInfo *info)
{
    int currentFile = 1;
    int overall_status = 0;
    bool first_msg = true;

    QString directory = KFileDialog::getExistingDirectory( QDir::homeDirPath(), info->parent() );

    //qDebug("starting by looking in directory: %s", directory.latin1());
    if ( directory.isEmpty() ) {
        info->addLog( i18n("No directory selected."));
        info->addLog( i18n("No files found for import."));
    } else {
        info->setOverall(0);
        traverseDirectory(info, directory);

        for ( QStringList::Iterator filename = mMboxFiles.begin(); filename != mMboxFiles.end(); ++filename, ++currentFile) {
            if ( info->shouldTerminate() ) break;
            QFile mbox( *filename );
            if (! mbox.open( IO_ReadOnly ) ) {
                info->alert( i18n("Unable to open %1, skipping").arg( *filename ) );
            } else {
                QFileInfo filenameInfo( *filename );
                kdDebug() << "importing filename " << *filename << endl;
                QStringList name = QStringList::split("/", *filename);
                QString folderName(name[name.count() - 2]);

                info->setCurrent(0);
                info->addLog( i18n("Importing emails from %1...").arg( *filename ) );
                info->setFrom( *filename );
                info->setTo( folderName );

                QByteArray input(MAX_LINE);
                long l = 0;

                while ( ! mbox.atEnd() ) {
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
                    l = mbox.readLine( input.data(),MAX_LINE); // read the first line, prevent "From "
                    tmp.file()->writeBlock( input, l );

                    while ( ! mbox.atEnd() &&  (l = mbox.readLine(input.data(),MAX_LINE)) && ((seperate = input.data()).left(5) != "From ")) {
                        tmp.file()->writeBlock( input, l );
                    }
                    tmp.close();
                    first_msg = false;

                    // force stop if user chancel the import
                    if ( info->shouldTerminate() ) {
                    tmp.unlink(); 
                    break;
                    }

                    /* comment by Danny Kukawka:
                    * addMessage() == old function, need more time and check for duplicates
                    * addMessage_fastImport == new function, faster and no check for duplicates
                    */
                    if(info->removeDupMsg)
                        addMessage( info, folderName, tmp.name() );
                    else
                        addMessage_fastImport( info, folderName, tmp.name() );

                    tmp.unlink();

                    int currentPercentage = (int) ( ( (float) mbox.at() / filenameInfo.size() ) * 100 );
                    info->setCurrent( currentPercentage );
                    if (currentFile == 1)
                        overall_status = (int)( currentPercentage*((float)currentFile/mMboxFiles.count()));
                    else
                        overall_status = (int)(((currentFile-1)*(100.0/(float)mMboxFiles.count()))+(currentPercentage*(1.0/(float)mMboxFiles.count())));
                    info->setOverall( overall_status );
                    if ( info->shouldTerminate() ) break;
                }

                info->addLog( i18n("Finished importing emails from %1").arg( *filename ) );
                if (count_duplicates > 0) {
                    info->addLog( i18n("1 duplicate message not imported to folder %1 in KMail", 
                                    "%n duplicate messages not imported to folder %1 in KMail", count_duplicates).arg(folderName));
                }
                count_duplicates = 0;
                mbox.close();
            }
        }
    }
    
    if (info->shouldTerminate()) info->addLog( i18n("Finished import, canceled by user."));
    info->setCurrent(100);
    info->setOverall(100);
}

void FilterMailApp::traverseDirectory(FilterInfo *info, const QString &dirName)
{
    QDir dir(dirName);
    dir.setFilter(QDir::Dirs | QDir::Files);

    const QFileInfoList *fileinfolist = dir.entryInfoList();
    QFileInfoListIterator it(*fileinfolist);
    QFileInfo *fi;

    if ( info->shouldTerminate() ) return;

    while ((fi = it.current())) {
        if (fi->fileName() == "." || fi->fileName() == "..") {
            ++it;
            continue;
        }
        if (fi->isDir() && fi->isReadable()) {
            traverseDirectory(info, fi->filePath());
        } else {
            if (!fi->isDir() && fi->fileName() == "mbox") {
                kdDebug() << "adding the file " << fi->filePath() << endl;
                mMboxFiles.append(fi->filePath());
            }
        }
        ++it;
    }
}
