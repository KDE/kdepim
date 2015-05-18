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

PimSettingExporterConsole::PimSettingExporterConsole(QObject *parent)
    : QObject(parent),
      mPimSettingsBackupRestore(new PimSettingsBackupRestore(this)),
      mMode(Import),
      mInProgress(false)
{

}

PimSettingExporterConsole::~PimSettingExporterConsole()
{

}

void PimSettingExporterConsole::start()
{

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
    mLogFileName = logFileName;
}

void PimSettingExporterConsole::setTemplateFileName(const QString &templateFileName)
{
    if (mInProgress) {
        qCDebug(PIMSETTINGEXPORTERCONSOLE_LOG) << "Already in progress. We can't change it.";
        return;
    }
    mTemplateFileName = templateFileName;
}




