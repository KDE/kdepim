/***************************************************************************
             filter_lnotes.cxx  -  Lotus Notes Structured Text mail import
                             -------------------
    begin                : Wed Feb 16, 2005
    copyright            : (C) 2005 by Robert Rockers
    email                : kconfigure@rockerssoft.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <KLocalizedString>
#include <kfiledialog.h>
#include <QTemporaryFile>
#include <qdebug.h>
#include <QFileInfo>

#include "filter_lnotes.h"
using namespace MailImporter;

/** Default constructor. */
FilterLNotes::FilterLNotes() :
    Filter(i18n("Import Lotus Notes Emails"),
           "Robert Rockers",
           i18n("<p><b>Lotus Notes Structured Text mail import filter</b></p>"
                "<p>This filter will import Structure Text files from an exported Lotus Notes email "
                "client into KMail. Use this filter if you want to import mails from Lotus or other "
                "mailers that use Lotus Notes' Structured Text format.</p>"
                "<p><b>Note:</b> Since it is possible to recreate the folder structure, the imported "
                "messages will be stored in subfolders named by the files they came from under: "
                "\"LNotes-Import\" in your local folder.</p>"))
{
    currentFile = 1;
    totalFiles = 0;
}

/** Destructor. */
FilterLNotes::~FilterLNotes()
{
}

/**
 * Recursive import of The Bat! maildir.
 * @param info Information storage for the operation.
 */
void FilterLNotes::import()
{

    const QStringList filenames = KFileDialog::getOpenFileNames(QDir::homePath(), "*|" + i18n("All Files (*)"),
                                  filterInfo()->parent());
    if (filenames.isEmpty()) {
        filterInfo()->alert(i18n("No files selected."));
        return;
    }

    currentFile = 1;
    totalFiles = 0;

    totalFiles = filenames.count();
    filterInfo()->setOverall(0);

    // See filter_mbox.cxx for better reference.
    QStringList::ConstIterator end = filenames.constEnd();
    for (QStringList::ConstIterator filename = filenames.constBegin(); filename != end; ++filename) {

        ++currentFile;
        filterInfo()->addInfoLogEntry(i18n("Importing emails from %1", *filename));
        ImportLNotes(*filename);
        filterInfo()->setOverall(100 * currentFile / totalFiles);
        if (filterInfo()->shouldTerminate()) {
            break;
        }
    }
}

/**
 * Import the files within a Folder.
 * @param file The name of the file to import.
 */
void FilterLNotes::ImportLNotes(const QString &file)
{

    // See Filter_pmail.cxx for better reference

    // Format of a Lotus Notes 5 Structured Text Document w form feed
    // Each email begins with a custom Header Principal:
    // The Message ends with a 0c character

    // open the message
    QFile f(file);

    if (! f.open(QIODevice::ReadOnly)) {
        filterInfo()->alert(i18n("Unable to open %1, skipping", file));
    } else {

        char ch = 0;
        int state = 0;
        int n = 0;
        QTemporaryFile *tempfile = 0;

        // Get folder name
        QFileInfo filenameInfo(file);
        QString folder(i18nc("Define folder name where we import lotus note mails", "LNotes-Import") + QLatin1Char('/') + filenameInfo.completeBaseName());
        filterInfo()->setTo(folder);

        // State machine to read the data in. The fgetc usage is probably terribly slow ...
        while (f.getChar(&ch)) {
            switch (state) {
            // new message state
            case 0:
                // open temp output file
                state = 1;
                filterInfo()->setCurrent(i18n("Message %1", n++));
                if (filterInfo()->shouldTerminate()) {
                    return;
                }

                tempfile = new QTemporaryFile;
                tempfile->setAutoRemove(false);
                tempfile->open();
            // fall through

            // inside a message state
            case 1:
                if (ch == 0x0c) {
                    // close file, send it
                    tempfile->close();

                    if (filterInfo()->removeDupMessage()) {
                        addMessage(folder, tempfile->fileName());
                    } else {
                        addMessage_fastImport(folder, tempfile->fileName());
                    }

                    tempfile->setAutoRemove(true);
                    state = 0;

                    int currentPercentage = (int)(((float) f.pos() / filenameInfo.size()) * 100);
                    filterInfo()->setCurrent(currentPercentage);
                    if (filterInfo()->shouldTerminate()) {
                        return;
                    }

                    break;
                }
                if (ch == 0x0d) {
                    break;
                }
                tempfile->putChar(ch);
                break;
            }
        }

        if (tempfile) {
            tempfile->close();
        }

        // did Folder end without 0x1a at the end?
        if (state != 0) {
            Q_ASSERT(tempfile);

            if (filterInfo()->removeDupMessage()) {
                addMessage(folder, tempfile->fileName());
            } else {
                addMessage_fastImport(folder, tempfile->fileName());
            }
        }
        if (tempfile) {
            tempfile->setAutoRemove(true);
            delete tempfile;
        }

        f.close();
    }
}

