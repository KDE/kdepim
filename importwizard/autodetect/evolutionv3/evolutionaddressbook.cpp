/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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
#include "evolutionaddressbook.h"
#include "importwizard.h"
#include <QProcess>
#include <KMessageBox>
#include <KLocalizedString>
#include <QFileDialog>

EvolutionAddressBook::EvolutionAddressBook(ImportWizard *parent)
    : AbstractAddressBook(parent)
{
    exportEvolutionAddressBook();
}

EvolutionAddressBook::~EvolutionAddressBook()
{

}

void EvolutionAddressBook::exportEvolutionAddressBook()
{
    KMessageBox::information(mImportWizard, i18n("Evolution address book will be exported as vCard. Import vCard in KAddressBook."), i18n("Export Evolution Address Book"));

    const QString directory = QFileDialog::getExistingDirectory(mImportWizard, i18n("Select the directory where vCards will be stored."));
    if (directory.isEmpty()) {
        return;
    }
    QFile evolutionFile;
    bool found = false;
    for (int i = 0; i < 9; ++i) {
        evolutionFile.setFileName(QStringLiteral("/usr/lib/evolution/3.%1/evolution-addressbook-export").arg(i));
        if (evolutionFile.exists()) {
            found = true;
            break;
        }
    }
    if (found) {
        QStringList arguments;
        arguments << QStringLiteral("-l");
        QProcess proc;
        proc.start(evolutionFile.fileName(), arguments);
        if (!proc.waitForFinished()) {
            return;
        }
        QByteArray result = proc.readAll();
        proc.close();
        if (!result.isEmpty()) {
            result = result.replace('\n', ',');
            const QString value(result.trimmed());
            const QStringList listAddressBook = value.split(QLatin1Char(','));
            //qCDebug(IMPORTWIZARD_LOG)<<" listAddressBook"<<listAddressBook;
            int i = 0;
            QString name;
            QString displayname;
            Q_FOREACH (const QString &arg, listAddressBook) {
                switch (i) {
                case 0:
                    name = arg;
                    name = name.remove(0, 1);
                    name = name.remove(name.length() - 1, 1);
                    ++i;
                    //name
                    break;
                case 1:
                    displayname = arg;
                    displayname = displayname.remove(0, 1);
                    displayname = displayname.remove(displayname.length() - 1, 1);
                    //display name
                    ++i;
                    break;
                case 2:
                    if (!displayname.isEmpty() && !name.isEmpty()) {
                        arguments.clear();
                        arguments << QStringLiteral("--format=vcard") << name << QStringLiteral("--output=%1/%2.vcard").arg(directory, displayname);
                        proc.start(evolutionFile.fileName(), arguments);
                        if (proc.waitForFinished()) {
                            addAddressBookImportInfo(i18n("Address book \"%1\" exported.", displayname));
                        } else {
                            addAddressBookImportError(i18n("Failed to export address book \"%1\".", displayname));
                        }
                    }
                    i = 0; //reset
                    break;
                }
            }
        }
    }
}

