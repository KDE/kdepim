/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>
  
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

#include "exportnotesjob.h"

#include "messageviewer/utils/kcursorsaver.h"

#include <Akonadi/AgentManager>

#include <KLocale>
#include <KStandardDirs>
#include <KTemporaryFile>
#include <KConfigGroup>
#include <KZip>

#include <QWidget>
#include <QFile>
#include <QDir>

ExportNotesJob::ExportNotesJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage,int numberOfStep)
    : AbstractImportExportJob(parent, archiveStorage, typeSelected, numberOfStep)
{
}

ExportNotesJob::~ExportNotesJob()
{

}

void ExportNotesJob::start()
{
    Q_EMIT title(i18n("Start export knotes settings..."));
    mArchiveDirectory = archive()->directory();
    if (mTypeSelected & Utils::Config) {
        backupConfig();
        increaseProgressDialog();
        if (wasCanceled()) {
            return;
        }
    }
    if (mTypeSelected & Utils::Data) {
        backupData();
        increaseProgressDialog();
        if (wasCanceled()) {
            return;
        }
    }
}

void ExportNotesJob::backupConfig()
{
    showInfo(i18n("Backing up config..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    const QString knotesStr(QLatin1String("knotesrc"));
    const QString knotesrc = KStandardDirs::locateLocal( "config", knotesStr);
    backupFile(knotesrc, Utils::configsPath(), knotesStr);

    Q_EMIT info(i18n("Config backup done."));
}

void ExportNotesJob::backupData()
{
    showInfo(i18n("Backing up data..."));
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
    const QString icsfileStr = QLatin1String( "notes.ics" );
    const QString icsfile = KStandardDirs::locateLocal( "data", QLatin1String( "knotes/" ) + icsfileStr );

    backupFile(icsfile, Utils::dataPath() +  QLatin1String( "/knotes/" ), icsfileStr);


    const QString notesDir = KStandardDirs::locateLocal( "data", QLatin1String( "knotes/notes/" ) );
    QDir notesDirectory( notesDir );
    if (notesDirectory.exists()) {
        const bool notesDirAdded = archive()->addLocalDirectory(notesDir, Utils::dataPath() +  QLatin1String( "/knotes/notes/" ));
        if (!notesDirAdded) {
            Q_EMIT error(i18n("\"%1\" directory cannot be added to backup file.", notesDir));
        }
    }
    const QString notesThemeDir = KStandardDirs::locateLocal( "data", QLatin1String( "knotes/print/" ) );
    QDir notesThemeDirectory( notesThemeDir );
    if (notesThemeDirectory.exists()) {
        const bool notesDirAdded = archive()->addLocalDirectory(notesDir, Utils::dataPath() +  QLatin1String( "/knotes/print" ));
        if (!notesDirAdded) {
            Q_EMIT error(i18n("\"%1\" directory cannot be added to backup file.", notesDir));
        }
    }
    Q_EMIT info(i18n("Data backup done."));
}

