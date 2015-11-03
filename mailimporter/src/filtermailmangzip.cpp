/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include <KLocalizedString>
#include <qfiledialog.h>
#include <QTemporaryFile>
#include "mailimporter_debug.h"
#include <KFilterDev>

#include "filtermailmangzip.h"

using namespace MailImporter;

FilterMailmanGzip::FilterMailmanGzip() :
    Filter(i18n("Import mailman gzip Files"),
           QStringLiteral("Laurent Montel"),
           i18n("<p><b>mailman gzip import filter</b></p>"
                "<p>This filter will import mailman gzipped files into KMail.</p>"
                "<p><b>Note:</b> Emails will be imported into folders named after the "
                "file they came from, prefixed with MAILMAN-</p>"))
{
}

FilterMailmanGzip::~FilterMailmanGzip()
{
}

void FilterMailmanGzip::import()
{
    const QStringList filenames = QFileDialog::getOpenFileNames(filterInfo()->parent(), QString(), QDir::homePath(), i18n("gzip Files (*.txt.gz)"));
    if (filenames.isEmpty()) {
        filterInfo()->alert(i18n("No files selected."));
        return;
    }
    importMails(filenames);
}

void FilterMailmanGzip::importMails(const QStringList &filenames)
{
    int currentFile = 1;
    int overall_status = 0;
    bool first_msg = true;

    filterInfo()->setOverall(0);
    QStringList::ConstIterator end(filenames.constEnd());
    for (QStringList::ConstIterator filename = filenames.constBegin(); filename != end; ++filename, ++currentFile) {
        QIODevice *device = new KCompressionDevice(*filename, KCompressionDevice::GZip);
        if (!device) {
            qCDebug(MAILIMPORTER_LOG) << "Could not create KFilterDev";
            filterInfo()->alert(i18n("Unable to open archive file '%1', skipping", *filename));
            continue;
        }

        device->open(QIODevice::ReadOnly);
        QFileInfo filenameInfo(*filename);
        QString folderName(QLatin1String("MAILMAN-") + filenameInfo.completeBaseName());

        filterInfo()->setCurrent(0);
        filterInfo()->addInfoLogEntry(i18n("Importing emails from %1...", *filename));

        filterInfo()->setFrom(*filename);
        filterInfo()->setTo(folderName);

        QByteArray input(MAX_LINE, '\0');
        long l = 0;

        while (! device->atEnd()) {
            QTemporaryFile tmp;
            tmp.open();
            qint64 filepos = 0;
            /* comment by Danny:
            * Don't use QTextStream to read from mbox, better use QDataStream. QTextStream only
            * support Unicode/Latin1/Locale. So you lost information from emails with
            * charset!=Unicode/Latin1/Locale (e.g. KOI8-R) and Content-Transfer-Encoding != base64
            * (e.g. 8Bit). It also not help to convert the QTextStream to Unicode. By this you
            * get Unicode/UTF-email but KMail can't detect the correct charset.
            */
            QByteArray separate;

            /* check if the first line start with "From " (and not "From: ") and discard the line
            * in this case because some IMAP servers (e.g. Cyrus) don't accept this header line */
            if (!first_msg && ((separate = input.data()).left(5) !=  "From ")) {
                tmp.write(input, l);
            }

            l = device->readLine(input.data(), MAX_LINE); // read the first line, prevent "From "

            if ((separate = input.data()).left(5) != "From ") {
                separate.replace(" at ", "@");
                tmp.write(separate, separate.length());
            }

            while (! device->atEnd() && (l = device->readLine(input.data(), MAX_LINE)) && ((separate = input.data()).left(5) != "From ")) {
                tmp.write(input, l);

                // workaround to fix hang if a corrupted mbox contains some
                // binary data, for more see bug #106796
                if (device->pos() == filepos) {
                    device->seek(device->size());
                } else {
                    filepos = device->pos();
                }
            }
            tmp.flush();
            first_msg = false;

            /* comment by Danny Kukawka:
            * addMessage() == old function, need more time and check for duplicates
            * addMessage_fastImport == new function, faster and no check for duplicates
            */
            if (tmp.size() > 0) {
                if (filterInfo()->removeDupMessage()) {
                    addMessage(folderName, tmp.fileName());
                } else {
                    addMessage_fastImport(folderName, tmp.fileName());
                }
            } else {
                qCWarning(MAILIMPORTER_LOG) << "Message size is 0 bytes, not importing it.";
            }

            int currentPercentage = (int)(((float) device->pos() / filenameInfo.size()) * 100);
            filterInfo()->setCurrent(currentPercentage);
            if (currentFile == 1) {
                overall_status = (int)(currentPercentage * ((float)currentFile / filenames.count()));
            } else {
                overall_status = (int)(((currentFile - 1) * (100.0 / (float)filenames.count())) + (currentPercentage * (1.0 / (float)filenames.count())));
            }
            filterInfo()->setOverall(overall_status);

            if (filterInfo()->shouldTerminate()) {
                break;
            }
        }

        filterInfo()->addInfoLogEntry(i18n("Finished importing emails from %1", *filename));
        if (countDuplicates() > 0) {
            filterInfo()->addInfoLogEntry(i18np("1 duplicate message not imported to folder %2 in KMail",
                                                "%1 duplicate messages not imported to folder %2 in KMail",
                                                countDuplicates(), folderName));
        }
        if (filterInfo()->shouldTerminate()) {
            filterInfo()->addInfoLogEntry(i18n("Finished import, canceled by user."));
        }

        setCountDuplicates(0);
        // don't forget to close the file !!!
        device->close();
        delete device;
    }
}
