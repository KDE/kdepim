/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>
  
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

#include "importnotesjob.h"
#include "archivestorage.h"

#include "pimcommon/util/createresource.h"

#include <KArchive>
#include <KLocalizedString>
#include <KGlobal>
#include <KStandardDirs>
#include <KZip>
#include <KConfigGroup>

#include <QFile>

ImportNotesJob::ImportNotesJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
    initializeImportJob();
}

ImportNotesJob::~ImportNotesJob()
{
}

void ImportNotesJob::start()
{
    Q_EMIT title(i18n("Start import KNotes settings..."));
    mArchiveDirectory = archive()->directory();
    if (mTypeSelected & Utils::Config)
        restoreConfig();
    if (mTypeSelected & Utils::Data)
        restoreData();
    Q_EMIT jobFinished();
}

void ImportNotesJob::restoreConfig()
{
    const QString knotesStr(QLatin1String("knotesrc"));
    restoreConfigFile(knotesStr);
    if (archiveVersion() <= 1) {
        const QString globalNoteSettingsStr(QLatin1String("globalnotesettings"));
        restoreConfigFile(globalNoteSettingsStr);
    } else {
        const QString globalNoteStr(QLatin1String("globalnotesettings"));
        const KArchiveEntry* globalNotecentry  = mArchiveDirectory->entry(Utils::configsPath() + globalNoteStr);
        if (globalNotecentry && globalNotecentry->isFile()) {
            const KArchiveFile* globalNotecentryrc = static_cast<const KArchiveFile*>(globalNotecentry);
            const QString globalNoterc = KStandardDirs::locateLocal( "config", globalNoteStr);
            if (QFile(globalNoterc).exists()) {
                if (overwriteConfigMessageBox(globalNoteStr)) {
                    importKNoteGlobalSettings(globalNotecentryrc,globalNoterc,globalNoteStr,Utils::configsPath());
                }
            } else {
                importKNoteGlobalSettings(globalNotecentryrc,globalNoterc,globalNoteStr,Utils::configsPath());
            }
        }

    }

    Q_EMIT info(i18n("Config restored."));
}

void ImportNotesJob::restoreData()
{
    if (archiveVersion() <= 1) {
        //Knote < knote-akonadi
        const KArchiveEntry *notesEntry  = mArchiveDirectory->entry(Utils::dataPath() + QLatin1String( "knotes/" ) );
        if (notesEntry && notesEntry->isDirectory()) {
            const QString notesPath = KGlobal::dirs()->saveLocation("data", QLatin1String("knotes/"));
            overwriteDirectory(notesPath, notesEntry);
        }
    }
    Q_EMIT info(i18n("Data restored."));
}


void ImportNotesJob::importKNoteGlobalSettings(const KArchiveFile* kmailsnippet, const QString& kmail2rc, const QString&filename,const QString& prefix)
{
    copyToFile(kmailsnippet,kmail2rc,filename,prefix);
    KSharedConfig::Ptr kmailConfig = KSharedConfig::openConfig(kmail2rc);

    const QString composerStr(QLatin1String("SelectNoteFolder"));
    if (kmailConfig->hasGroup(composerStr)) {
        KConfigGroup composerGroup = kmailConfig->group(composerStr);
        const QString previousStr(QLatin1String("DefaultFolder"));
        convertRealPathToCollection(composerGroup, previousStr);
    }
    kmailConfig->sync();
}
