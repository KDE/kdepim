/*
  This file is part of KAddressbook.
  Copyright (c) 2000 - 2002 Oliver Strutynski <olistrut@gmx.de>
  Copyright (c) 2002 - 2003 Helge Deller <deller@kde.org>
                            Tobias Koenig <tokoe@kde.org>

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

/*
  Description:
  The LDAP Data Interchange Format (LDIF) is a common ASCII-text based
  Internet interchange format. Most programs allow you to export data in
  LDIF format and e.g. Netscape and Mozilla store by default their
  Personal Address Book files in this format.
  This import and export filter reads and writes any LDIF version 1 files
  into your KDE Addressbook.
*/

#include "ldif_xxport.h"

#include "pimcommon/widgets/renamefiledialog.h"

#include <KContacts/LDIFConverter>

#include <KLocalizedString>
#include <KMessageBox>
#include <QTemporaryFile>
#include <KIO/NetAccess>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QFileDialog>

void doExport(QFile *file, const ContactList &list)
{
    QString data;
    KContacts::LDIFConverter::addresseeAndContactGroupToLDIF(list.addressList(), list.contactGroupList(), data);

    QTextStream stream(file);
    stream.setCodec("UTF-8");
    stream << data;
}

LDIFXXPort::LDIFXXPort(QWidget *parentWidget)
    : XXPort(parentWidget)
{
}

ContactList LDIFXXPort::importContacts() const
{
    ContactList contactList;
    const QString fileName = QFileDialog::getOpenFileName(parentWidget() , QString(),  QDir::homePath(), i18n("LDIF file (%1)", QStringLiteral("*.ldif")));
    if (fileName.isEmpty()) {
        return contactList;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString msg = i18n("<qt>Unable to open <b>%1</b> for reading.</qt>", fileName);
        KMessageBox::error(parentWidget(), msg);
        return contactList;
    }

    QTextStream stream(&file);
    stream.setCodec("ISO 8859-1");

    const QString wholeFile = stream.readAll();
    const QDateTime dtDefault = QFileInfo(file).lastModified();
    file.close();
    KContacts::ContactGroup::List lstGroup;
    KContacts::Addressee::List lstAddresses;
    KContacts::LDIFConverter::LDIFToAddressee(wholeFile, lstAddresses, lstGroup, dtDefault);
    contactList.setAddressList(lstAddresses);
    contactList.setContactGroupList(lstGroup);
    return contactList;
}

bool LDIFXXPort::exportContacts(const ContactList &list, VCardExportSelectionWidget::ExportFields) const
{
    const QUrl url =
        QFileDialog::getSaveFileUrl(parentWidget(), QString(), QUrl::fromLocalFile(QDir::homePath() + QLatin1String("/addressbook.ldif")), QLatin1String("text/x-ldif"));
    if (url.isEmpty()) {
        return true;
    }

    if (!url.isLocalFile()) {
        QTemporaryFile tmpFile;
        if (!tmpFile.open()) {
            const QString msg = i18n("<qt>Unable to open file <b>%1</b></qt>", url.url());
            KMessageBox::error(parentWidget(), msg);
            return false;
        }

        doExport(&tmpFile, list);
        tmpFile.flush();

        return KIO::NetAccess::upload(tmpFile.fileName(), url, parentWidget());
    } else {
        QString fileName = url.toLocalFile();

        if (QFileInfo(fileName).exists()) {
            if (url.isLocalFile() && QFileInfo(url.toLocalFile()).exists()) {
                PimCommon::RenameFileDialog::RenameFileDialogResult result = PimCommon::RenameFileDialog::RENAMEFILE_IGNORE;
                PimCommon::RenameFileDialog *dialog = new PimCommon::RenameFileDialog(url, false, parentWidget());
                result = static_cast<PimCommon::RenameFileDialog::RenameFileDialogResult>(dialog->exec());
                if (result == PimCommon::RenameFileDialog::RENAMEFILE_RENAME) {
                    fileName = dialog->newName().toLocalFile();
                } else if (result == PimCommon::RenameFileDialog::RENAMEFILE_IGNORE) {
                    delete dialog;
                    return true;
                }
                delete dialog;
            }
        }

        QFile file(fileName);

        if (!file.open(QIODevice::WriteOnly)) {
            QString txt = i18n("<qt>Unable to open file <b>%1</b>.</qt>", fileName);
            KMessageBox::error(parentWidget(), txt);
            return false;
        }

        doExport(&file, list);
        file.close();

        return true;
    }
}
