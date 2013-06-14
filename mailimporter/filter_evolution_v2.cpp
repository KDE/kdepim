/***************************************************************************
            filter_evolution_v2.cxx  -  Evolution 2.0.x mail import
                             -------------------
    begin                : Januar 26 2005
    copyright            : (C) 2005 by Danny Kukawka <danny.kukawka@web.de>
                           (inspired and partly copied from filter_evolution)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "filter_evolution_v2.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <ktemporaryfile.h>
#include <QPointer>

using namespace MailImporter;

/** Default constructor. */
FilterEvolution_v2::FilterEvolution_v2() :
    Filter(i18n("Import Evolution 2.x Local Mails and Folder Structure"),
           "Danny Kukawka",
           i18n("<p><b>Evolution 2.x import filter</b></p>"
                "<p>Select the base directory of your local Evolution mailfolder (usually ~/.evolution/mail/local/).</p>"
                "<p><b>Note:</b> Never choose a Folder which <u>does not</u> contain mbox-files (for example "
                "a maildir): if you do, you will get many new folders.</p>"
                "<p>Since it is possible to recreate the folder structure, the folders "
                "will be stored under: \"Evolution-Import\".</p>"))
{
}

/** Destructor. */
FilterEvolution_v2::~FilterEvolution_v2()
{
}


QString FilterEvolution_v2::defaultSettingsPath()
{
    return QDir::homePath() + QLatin1String( "/.evolution/mail/local" );
}

/** Recursive import of Evolution's mboxes. */
void FilterEvolution_v2::import()
{
    setCountDuplicates(0);
    /**
   * We ask the user to choose Evolution's root directory.
   * This should be usually ~/.evolution/mail/local/
   */
    QString evolDir = defaultSettingsPath();
    QDir d( evolDir );
    if ( !d.exists() ) {
        evolDir = QDir::homePath();
    }

    QPointer<KFileDialog> kfd = new KFileDialog( evolDir, "", 0 );
    kfd->setMode(KFile::Directory | KFile::LocalOnly);
    if (kfd->exec()) {
        const QString dir = kfd->selectedFile();
        importMails( dir );
    }
    delete kfd;
}

bool FilterEvolution_v2::excludeFiles(const QString &file)
{
    if (( file.endsWith(QLatin1String(".db")) ||
          file.endsWith(QLatin1String(".cmeta")) ||
          file.endsWith(QLatin1String(".ev-summary")) ||
          file.endsWith(QLatin1String(".ibex.index")) ||
          file.endsWith(QLatin1String(".ibex.index.data")) )) {
        return true;
    }
    return false;
}

void FilterEvolution_v2::importMails( const QString &maildir )
{
    setMailDir( maildir );
    if (mailDir().isEmpty()) {
        filterInfo()->alert(i18n("No directory selected."));
        return;
    }
    /**
   * If the user only select homedir no import needed because
   * there should be no files and we surely import wrong files.
   */
    else if ( mailDir() == QDir::homePath() || mailDir() == (QDir::homePath() + '/')) {
        filterInfo()->addErrorLogEntry(i18n("No files found for import."));
    } else {
        filterInfo()->setOverall(0);

        /** Recursive import of the MailArchives */
        QDir dir(mailDir());
        const QStringList rootSubDirs = dir.entryList(QStringList("[^\\.]*"), QDir::Dirs, QDir::Name); // Removal of . and ..
        int currentDir = 1, numSubDirs = rootSubDirs.size();
        QStringList::ConstIterator endFilename( rootSubDirs.constEnd() );
        for (QStringList::ConstIterator filename = rootSubDirs.constBegin() ; filename != endFilename ; ++filename, ++currentDir) {
            if (filterInfo()->shouldTerminate())
                break;
            importDirContents(dir.filePath(*filename), *filename, *filename);
            filterInfo()->setOverall((int) ((float) currentDir / numSubDirs * 100));
        }

        /** import last but not least all archives from the root-dir */
        QDir importDir (mailDir());
        const QStringList files = importDir.entryList(QStringList("[^\\.]*"), QDir::Files, QDir::Name);
        endFilename = files.constEnd();
        for ( QStringList::ConstIterator mailFile = files.constBegin(); mailFile != endFilename; ++mailFile) {
            if (filterInfo()->shouldTerminate())
                break;
            QString temp_mailfile = *mailFile;
            if (!excludeFiles(temp_mailfile) )
            {
                filterInfo()->addInfoLogEntry( i18n("Start import file %1...", temp_mailfile ) );
                importMBox(mailDir() + temp_mailfile , temp_mailfile, QString());
            }
        }

        filterInfo()->addInfoLogEntry( i18n("Finished importing emails from %1", mailDir() ));
        if(countDuplicates() > 0) {
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
 * @param KMailRootDir The directory's root directory in KMail's folder structure.
 * @param KMailSubDir The directory's direct ancestor in KMail's folder structure.
 */
void FilterEvolution_v2::importDirContents(const QString &dirName, const QString &KMailRootDir, const QString &KMailSubDir)
{
    if (filterInfo()->shouldTerminate()) return;

    /** Here Import all archives in the current dir */
    QDir dir(dirName);

    QDir importDir (dirName);
    const QStringList files = importDir.entryList(QStringList("[^\\.]*"), QDir::Files, QDir::Name);
    QStringList::ConstIterator mailFileEnd( files.constEnd() );
    for ( QStringList::ConstIterator mailFile = files.constBegin(); mailFile != mailFileEnd; ++mailFile) {
        QString temp_mailfile = *mailFile;
        if (!excludeFiles(temp_mailfile))
        {
            filterInfo()->addInfoLogEntry( i18n("Start import file %1...", temp_mailfile ) );
            importMBox((dirName + '/' + temp_mailfile) , KMailRootDir, KMailSubDir);
        }
    }

    /** If there are subfolders, we import them one by one */
    QDir subfolders(dirName);
    const QStringList subDirs = subfolders.entryList(QStringList("[^\\.]*"), QDir::Dirs, QDir::Name);
    QStringList::ConstIterator end( subDirs.constEnd() );
    for (QStringList::ConstIterator filename = subDirs.constBegin() ; filename != end; ++filename) {
        QString kSubDir;
        if(!KMailSubDir.isNull()) {
            kSubDir = KMailSubDir + '/' + *filename;
        } else {
            kSubDir = *filename;
        }
        importDirContents( subfolders.filePath(*filename), KMailRootDir, kSubDir);
    }
}

/**
 * Import of a MBox file.
 * @param info Information storage for the operation.
 * @param dirName The MBox's name.
 * @param KMailRootDir The directory's root directory in KMail's folder structure.
 * @param KMailSubDir The directory's equivalent in KMail's folder structure. *
 */
void FilterEvolution_v2::importMBox(const QString &mboxName, const QString &rootDir, const QString &targetDir)
{
    QFile mbox(mboxName);
    bool first_msg = true;
    if (!mbox.open(QIODevice::ReadOnly)) {
        filterInfo()->alert(i18n("Unable to open %1, skipping", mboxName));
    } else {
        QFileInfo filenameInfo(mboxName);

        filterInfo()->setCurrent(0);
        if( mboxName.length() > 20 ) {
            QString tmp_info = mboxName;
            tmp_info = tmp_info.replace( mailDir(), "../" );
            if (tmp_info.contains(".sbd"))
                tmp_info.remove(".sbd");
            filterInfo()->setFrom( tmp_info );
        } else
            filterInfo()->setFrom(mboxName);
        
        if(targetDir.contains(QLatin1String( ".sbd" ))) {
            QString tmp_info = targetDir;
            tmp_info.remove(QLatin1String( ".sbd" ));
            filterInfo()->setTo(tmp_info);
        } else
            filterInfo()->setTo(targetDir);

        QByteArray input(MAX_LINE, '\0');
        long l = 0;

        while (!mbox.atEnd()) {
            KTemporaryFile tmp;
            tmp.open();
            /** @todo check if the file is really a mbox, maybe search for 'from' string at start */
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

            QString destFolder;
            QString _targetDir = targetDir;
            if(!targetDir.isNull()) {
                if(_targetDir.contains(QLatin1String( ".sbd" )))
                    _targetDir.remove(QLatin1String( ".sbd" ));
                destFolder += "Evolution-Import/" + _targetDir + '/' + filenameInfo.completeBaseName(); // mboxName;
            } else {
                destFolder = "Evolution-Import/" + rootDir;
                if(destFolder.contains(QLatin1String( ".sbd" )))
                    destFolder.remove(QLatin1String( ".sbd" ));
            }


            if(filterInfo()->removeDupMessage())
                addMessage( destFolder, tmp.fileName() );
            else
                addMessage_fastImport( destFolder, tmp.fileName() );

            int currentPercentage = (int) (((float) mbox.pos() / filenameInfo.size()) * 100);
            filterInfo()->setCurrent(currentPercentage);
            if (filterInfo()->shouldTerminate())
                break;
        }
        mbox.close();
    }
}

