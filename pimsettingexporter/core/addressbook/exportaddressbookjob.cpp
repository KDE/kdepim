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

#include "exportaddressbookjob.h"
#include "messageviewer/utils/kcursorsaver.h"

#include <AkonadiCore/AgentManager>

#include <KLocalizedString>

#include <QTemporaryFile>
#include <KConfigGroup>
#include <KZip>

#include <QWidget>
#include <QDir>
#include <QStandardPaths>

ExportAddressbookJob::ExportAddressbookJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
}

ExportAddressbookJob::~ExportAddressbookJob()
{

}

void ExportAddressbookJob::start()
{
    Q_EMIT title(i18n("Start export KAddressBook settings..."));
    mArchiveDirectory = archive()->directory();
    if (mTypeSelected & Utils::Resources) {
        backupResources();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
    if (mTypeSelected & Utils::Config) {
        backupConfig();
        increaseProgressDialog();
        if (wasCanceled()) {
            Q_EMIT jobFinished();
            return;
        }
    }
    Q_EMIT jobFinished();
}

void ExportAddressbookJob::backupResources()
{
    showInfo(i18n("Backing up resources..."));
    MessageViewer::KCursorSaver busy(MessageViewer::KBusyPtr::busy());
    Akonadi::AgentManager *manager = Akonadi::AgentManager::self();
    const Akonadi::AgentInstance::List list = manager->instances();
    foreach (const Akonadi::AgentInstance &agent, list) {
        const QString identifier = agent.identifier();
        if (identifier.contains(QStringLiteral("akonadi_vcarddir_resource_")) || identifier.contains(QStringLiteral("akonadi_contacts_resource_"))) {
            const QString archivePath = Utils::addressbookPath() + identifier + QDir::separator();

            QUrl url = Utils::resourcePath(agent, QStringLiteral("$HOME/.local/share/contacts/"));
            if (!mAgentPaths.contains(url.path())) {
                mAgentPaths << url.path();
                if (!url.isEmpty()) {
                    const bool fileAdded = backupFullDirectory(url, archivePath, QStringLiteral("addressbook.zip"));
                    if (fileAdded) {
                        const QString errorStr = Utils::storeResources(archive(), identifier, archivePath);
                        if (!errorStr.isEmpty()) {
                            Q_EMIT error(errorStr);
                        }
                        url = Utils::akonadiAgentConfigPath(identifier);
                        if (!url.isEmpty()) {
                            const QString filename = url.fileName();
                            const bool fileAdded  = archive()->addLocalFile(url.path(), archivePath + filename);
                            if (fileAdded) {
                                Q_EMIT info(i18n("\"%1\" was backed up.", filename));
                            } else {
                                Q_EMIT error(i18n("\"%1\" file cannot be added to backup file.", filename));
                            }
                        }
                    }
                }
            }
        } else if (identifier.contains(QStringLiteral("akonadi_vcard_resource_"))) {
            backupResourceFile(agent, Utils::addressbookPath());
        }
    }
    Q_EMIT info(i18n("Resources backup done."));
}

void ExportAddressbookJob::backupConfig()
{
    showInfo(i18n("Backing up config..."));
    MessageViewer::KCursorSaver busy(MessageViewer::KBusyPtr::busy());

    const QString kaddressbookStr(QStringLiteral("kaddressbookrc"));
    const QString kaddressbookrc = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + kaddressbookStr;
    if (QFile(kaddressbookrc).exists()) {
        KSharedConfigPtr kaddressbook = KSharedConfig::openConfig(kaddressbookrc);

        QTemporaryFile tmp;
        tmp.open();

        KConfig *kaddressBookConfig = kaddressbook->copyTo(tmp.fileName());

        const QString collectionViewCheckStateStr(QStringLiteral("CollectionViewCheckState"));
        if (kaddressBookConfig->hasGroup(collectionViewCheckStateStr)) {
            KConfigGroup group = kaddressBookConfig->group(collectionViewCheckStateStr);
            const QString selectionKey(QStringLiteral("Selection"));
            Utils::convertCollectionListToRealPath(group, selectionKey);
        }

        const QString collectionViewStateStr(QStringLiteral("CollectionViewState"));
        if (kaddressBookConfig->hasGroup(collectionViewStateStr)) {
            KConfigGroup group = kaddressBookConfig->group(collectionViewStateStr);
            QString currentKey(QStringLiteral("Current"));
            Utils::convertCollectionToRealPath(group, currentKey);

            currentKey = QStringLiteral("Expansion");
            Utils::convertCollectionToRealPath(group, currentKey);

            currentKey = QStringLiteral("Selection");
            Utils::convertCollectionToRealPath(group, currentKey);
        }
        kaddressBookConfig->sync();
        backupFile(tmp.fileName(), Utils::configsPath(), kaddressbookStr);
        delete kaddressBookConfig;
    }
    Q_EMIT info(i18n("Config backup done."));
}

