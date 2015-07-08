/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "pimsettingexporterconsole.h"
#include "pimsettingsbackuprestore.h"
#include "pimsettingexportconsole_debug.h"
#include "loginfile.h"
#include "loginfo.h"
#include "xml/templateselection.h"

PimSettingExporterConsole::PimSettingExporterConsole(QObject *parent)
    : QObject(parent),
      mPimSettingsBackupRestore(new PimSettingsBackupRestore(this)),
      mLogInFile(new LogInFile(this)),
      mLogInfo(new LogInfo(this)),
      mMode(Import),
      mInProgress(false)
{
    initializeLogInFile();
}

PimSettingExporterConsole::~PimSettingExporterConsole()
{

}

void PimSettingExporterConsole::initializeLogInFile()
{
    connect(mPimSettingsBackupRestore, &PimSettingsBackupRestore::addEndLine, this, &PimSettingExporterConsole::slotAddEndLine);
    connect(mPimSettingsBackupRestore, &PimSettingsBackupRestore::addError, this, &PimSettingExporterConsole::slotAddError);
    connect(mPimSettingsBackupRestore, &PimSettingsBackupRestore::addInfo, this, &PimSettingExporterConsole::slotAddInfo);
    connect(mPimSettingsBackupRestore, &PimSettingsBackupRestore::addTitle, this, &PimSettingExporterConsole::slotAddTitle);
}

void PimSettingExporterConsole::slotAddEndLine()
{
    mLogInFile->addEndLine();
    mLogInfo->addEndLineLogEntry();
}

void PimSettingExporterConsole::slotAddError(const QString &message)
{
    mLogInFile->addError(message);
    mLogInfo->addErrorLogEntry(message);
}

void PimSettingExporterConsole::slotAddInfo(const QString &message)
{
    mLogInFile->addInfo(message);
    mLogInfo->addInfoLogEntry(message);
}

void PimSettingExporterConsole::slotAddTitle(const QString &message)
{
    mLogInFile->addTitle(message);
    mLogInfo->addTitleLogEntry(message);
}

QString PimSettingExporterConsole::importExportFileName() const
{
    return mImportExportFileName;
}

void PimSettingExporterConsole::setImportExportFileName(const QString &filename)
{
    if (mInProgress) {
        qCDebug(PIMSETTINGEXPORTERCONSOLE_LOG) << "Already in progress. We can't change it.";
        return;
    }
    mImportExportFileName = filename;
}

void PimSettingExporterConsole::start()
{
#if 0
    //Load template if necessary
    if (!mTemplateFileName.isEmpty()) {
        TemplateSelection selection(mTemplateFileName);
        const QHash<Utils::AppsType, Utils::StoredTypes> templateElements = selection.loadTemplate();
        mPimSettingsBackupRestore->setStoredParameters(templateElements);
    }
#endif
    //TODO
    switch (mMode) {
    case Import:
        break;
    case Export:
        break;
    }
}

PimSettingExporterConsole::Mode PimSettingExporterConsole::mode() const
{
    return mMode;
}

void PimSettingExporterConsole::setMode(const Mode &mode)
{
    if (mInProgress) {
        qCDebug(PIMSETTINGEXPORTERCONSOLE_LOG) << "Already in progress. We can't change it.";
        return;
    }
    mMode = mode;
}

void PimSettingExporterConsole::setLogFileName(const QString &logFileName)
{
    if (mInProgress) {
        qCDebug(PIMSETTINGEXPORTERCONSOLE_LOG) << "Already in progress. We can't change it.";
        return;
    }
    mLogInFile->setFileName(logFileName);
}

void PimSettingExporterConsole::setTemplateFileName(const QString &templateFileName)
{
    if (mInProgress) {
        qCDebug(PIMSETTINGEXPORTERCONSOLE_LOG) << "Already in progress. We can't change it.";
        return;
    }
    mTemplateFileName = templateFileName;
}
