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


#include "pimsettingsbackuprestoreui.h"
#include "importexportprogressindicatorgui.h"
#include "abstractimportexportjob.h"
#include <KMessageBox>
#include <KLocalizedString>

PimSettingsBackupRestoreUI::PimSettingsBackupRestoreUI(QWidget *parentWidget, QObject *parent)
    : PimSettingsBackupRestore(parent),
      mParentWidget(parentWidget)
{

}

PimSettingsBackupRestoreUI::~PimSettingsBackupRestoreUI()
{

}

bool PimSettingsBackupRestoreUI::continueToRestore()
{
    if (KMessageBox::No == KMessageBox::questionYesNo(mParentWidget, i18n("The archive was created by a newer version of this program. It might contain additional data which will be skipped during import. Do you want to import it?"), i18n("Not correct version")))
        return false;
    return true;
}

void PimSettingsBackupRestoreUI::addExportProgressIndicator()
{
    mImportExportData->setImportExportProgressIndicator(new ImportExportProgressIndicatorGui(mParentWidget, this));
}
