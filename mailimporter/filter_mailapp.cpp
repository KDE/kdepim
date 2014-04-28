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


#include <klocale.h>
#include <kfiledialog.h>
#include <ktemporaryfile.h>
#include <qdebug.h>

#include "filter_mailapp.h"

using namespace MailImporter;


FilterMailApp::FilterMailApp() :
    Filter( i18n("Import From OS X Mail"),
            "Chris Howells<br /><br />Filter accelerated by Danny Kukawka )",
            i18n("<p><b>OS X Mail Import Filter</b></p>"
                 "<p>This filter imports e-mails from the Mail client in Apple Mac OS X.</p>"))
{
}

FilterMailApp::~FilterMailApp()
{
}



void FilterMailApp::import()
{
    const QString directory = KFileDialog::getExistingDirectory( QDir::homePath(), filterInfo()->parent() );
    importMails( directory );
}

void FilterMailApp::importMails( const QString  &maildir )
{
    int currentFile = 1;
    int overall_status = 0;
    bool first_msg = true;

    setMailDir(maildir);
    if ( mailDir().isEmpty() )
    {
        filterInfo()->alert(i18n("No files selected."));
        return;
    }
    
    filterInfo()->setOverall(0);

    //   qDebug() <<"starting by looking in directory" << directory;
    traverseDirectory(mailDir());

    QStringList::ConstIterator end( mMboxFiles.constEnd() );
    for ( QStringList::ConstIterator filename = mMboxFiles.constBegin(); filename != end; ++filename, ++currentFile) {
        if ( filterInfo()->shouldTerminate() )
            break;
        QFile mbox( *filename );
        if (! mbox.open( QIODevice::ReadOnly ) ) {
            filterInfo()->alert( i18n("Unable to open %1, skipping", *filename ) );
        } else {
            QFileInfo filenameInfo( *filename );
            qDebug() <<"importing filename" << *filename;
            QStringList name = (*filename).split('/', QString::SkipEmptyParts);
            QString folderName(name[name.count() - 2]);

            filterInfo()->setCurrent(0);
            filterInfo()->addInfoLogEntry( i18n("Importing emails from %1...", *filename ) );
            filterInfo()->setFrom( *filename );
            filterInfo()->setTo( folderName );

            QByteArray input(MAX_LINE,'\0');
            long l = 0;

            while ( ! mbox.atEnd() ) {
                KTemporaryFile tmp;
                tmp.open();
                /* comment by Danny:
         * Don't use QTextStream to read from mbox, better use QDataStream. QTextStream only
         * support Unicode/Latin1/Locale. So you lost information from emails with
         * charset!=Unicode/Latin1/Locale (e.g. KOI8-R) and Content-Transfer-Encoding != base64
         * (e.g. 8Bit). It also not help to convert the QTextStream to Unicode. By this you
         * get Unicode/UTF-email but KMail can't detect the correct charset.
         */
                QByteArray separate;

                if(!first_msg)
                    tmp.write( input, l );
                l = mbox.readLine( input.data(),MAX_LINE); // read the first line, prevent "From "
                tmp.write( input, l );

                while ( ! mbox.atEnd() && (l = mbox.readLine(input.data(),MAX_LINE)) &&((separate = input.data()).left(5) != "From ")) {
                    tmp.write( input, l );
                }
                tmp.flush();
                first_msg = false;

                /* comment by Danny Kukawka:
         * addMessage() == old function, need more time and check for duplicates
         * addMessage_fastImport == new function, faster and no check for duplicates
         */
                if(filterInfo()->removeDupMessage())
                    addMessage( folderName, tmp.fileName() );
                else
                    addMessage_fastImport( folderName, tmp.fileName() );

                int currentPercentage = (int) ( ( (float) mbox.pos() / filenameInfo.size() ) * 100 );
                filterInfo()->setCurrent( currentPercentage );
                if (currentFile == 1)
                    overall_status = (int)( currentPercentage*((float)currentFile/mMboxFiles.count()));
                else
                    overall_status = (int)(((currentFile-1)*(100.0/(float)mMboxFiles.count()))+(currentPercentage*(1.0/(float)mMboxFiles.count())));
                filterInfo()->setOverall( overall_status );
                if ( filterInfo()->shouldTerminate() ) break;
            }

            filterInfo()->addInfoLogEntry( i18n("Finished importing emails from %1", *filename ) );
            if (countDuplicates() > 0) {
                filterInfo()->addInfoLogEntry( i18np("1 duplicate message not imported to folder %2 in KMail",
                                                     "%1 duplicate messages not imported to folder %2 in KMail", countDuplicates(), folderName));
            }
            setCountDuplicates(0);
            mbox.close();
        }
    }
    if (filterInfo()->shouldTerminate())
        filterInfo()->addInfoLogEntry( i18n("Finished import, canceled by user."));
}

void FilterMailApp::traverseDirectory(const QString &dirName)
{
    QDir dir(dirName);
    dir.setFilter(QDir::Dirs | QDir::Files);

    const QFileInfoList fileinfolist = dir.entryInfoList();
    Q_FOREACH( const QFileInfo &fi, fileinfolist ) {
        const QString filename(fi.fileName());
        if (filename == QLatin1String( "." ) || filename == QLatin1String( ".." ) ) {
            continue;
        }
        if (fi.isDir() && fi.isReadable()) {
            traverseDirectory(fi.filePath());
        } else {
            if (!fi.isDir() && (filename == QLatin1String( "mbox" ))) {
                qDebug() <<"adding the file" << fi.filePath();
                mMboxFiles.append(fi.filePath());
            }
        }
    }
}
