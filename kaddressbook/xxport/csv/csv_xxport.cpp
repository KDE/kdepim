/*
  This file is part of KAddressBook.
  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "csv_xxport.h"
#include "contactfields.h"
#include "csvimportdialog.h"

#include "PimCommon/RenameFileDialog"

#include <QFileDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <QTemporaryFile>
#include <QUrl>
#include <KJobWidgets>
#include <KIO/StatJob>
#include <KIO/FileCopyJob>

#include <QPointer>
#include <QTextCodec>
#include <QTextStream>

CsvXXPort::CsvXXPort(QWidget *parent)
    : XXPort(parent)
{
}

bool CsvXXPort::exportContacts(const ContactList &contacts, VCardExportSelectionWidget::ExportFields) const
{
    QUrl url = QFileDialog::getSaveFileUrl(parentWidget(), QString(), QUrl::fromUserInput(QStringLiteral("addressbook.csv")));
    if (url.isEmpty()) {
        return true;
    }

    if (QFileInfo(url.isLocalFile() ? url.toLocalFile() : url.path()).exists()) {
        if (url.isLocalFile() && QFileInfo(url.toLocalFile()).exists()) {
            PimCommon::RenameFileDialog::RenameFileDialogResult result = PimCommon::RenameFileDialog::RENAMEFILE_IGNORE;
            PimCommon::RenameFileDialog *dialog = new PimCommon::RenameFileDialog(url, false, parentWidget());
            result = static_cast<PimCommon::RenameFileDialog::RenameFileDialogResult>(dialog->exec());
            if (result == PimCommon::RenameFileDialog::RENAMEFILE_RENAME) {
                url = dialog->newName();
            } else if (result == PimCommon::RenameFileDialog::RENAMEFILE_IGNORE) {
                delete dialog;
                return true;
            }
            delete dialog;
        }
    }

    if (!url.isLocalFile()) {
        QTemporaryFile tmpFile;
        if (!tmpFile.open()) {
            const QString msg = i18n("<qt>Unable to open file <b>%1</b></qt>", url.url());
            KMessageBox::error(parentWidget(), msg);
            return false;
        }

        exportToFile(&tmpFile, contacts.addressList());
        tmpFile.flush();
        auto job = KIO::file_copy(QUrl::fromLocalFile(tmpFile.fileName()), url);
        KJobWidgets::setWindow(job, parentWidget());
        return job->exec();
    } else {
        QFile file(url.toLocalFile());
        if (!file.open(QIODevice::WriteOnly)) {
            const QString msg = i18n("<qt>Unable to open file <b>%1</b>.</qt>", url.toLocalFile());
            KMessageBox::error(parentWidget(), msg);
            return false;
        }

        exportToFile(&file, contacts.addressList());
        file.close();

        return true;
    }
}

void CsvXXPort::exportToFile(QFile *file, const KContacts::Addressee::List &contacts) const
{
    QTextStream stream(file);
    stream.setCodec(QTextCodec::codecForLocale());

    ContactFields::Fields fields = ContactFields::allFields();
    fields.remove(ContactFields::Undefined);

    bool first = true;

    // First output the column headings
    for (int i = 0; i < fields.count(); ++i) {
        if (!first) {
            stream << ",";
        }

        // add quoting as defined in RFC 4180
        QString label = ContactFields::label(fields.at(i));
        label.replace(QLatin1Char('"'), QStringLiteral("\"\""));

        stream << "\"" << label << "\"";
        first = false;
    }
    stream << "\n";

    // Then all the contacts
    for (int i = 0; i < contacts.count(); ++i) {

        const KContacts::Addressee contact = contacts.at(i);
        first = true;

        for (int j = 0; j < fields.count(); ++j) {
            if (!first) {
                stream << ",";
            }

            QString content;
            if (fields.at(j) == ContactFields::Birthday ||
                    fields.at(j) == ContactFields::Anniversary) {
                const QDateTime dateTime =
                    QDateTime::fromString(ContactFields::value(fields.at(j), contact), Qt::ISODate);
                if (dateTime.isValid()) {
                    content = dateTime.date().toString(Qt::ISODate);
                }
            } else {
                content = ContactFields::value(fields.at(j), contact).replace(QLatin1Char('\n'), QStringLiteral("\\n"));
            }

            // add quoting as defined in RFC 4180
            content.replace(QLatin1Char('"'), QStringLiteral("\"\""));

            stream << '\"' << content << '\"';
            first = false;
        }

        stream << "\n";
    }
}

ContactList CsvXXPort::importContacts() const
{
    ContactList contactList;
    QPointer<CSVImportDialog> dlg = new CSVImportDialog(parentWidget());
    if (dlg->exec() && dlg) {
        contactList.setAddressList(dlg->contacts());
    }

    delete dlg;
    return contactList;
}
