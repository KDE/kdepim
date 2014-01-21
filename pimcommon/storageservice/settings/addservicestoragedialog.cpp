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

#include "addservicestoragedialog.h"
#include "storageservicecombobox.h"

#include <KDialog>
#include <KLocalizedString>

using namespace PimCommon;

AddServiceStorageDialog::AddServiceStorageDialog(PimCommon::StorageServiceAbstract::Capability cap, const QStringList &excludeService, QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Add Service" ) );
    setButtons( Ok | Cancel );
    mService = new StorageServiceComboBox(cap, excludeService);
    setMainWidget(mService);
    enableButtonOk(mService->count() > 0);
}

AddServiceStorageDialog::~AddServiceStorageDialog()
{

}

PimCommon::StorageServiceManager::ServiceType AddServiceStorageDialog::serviceSelected() const
{
    return mService->service();
}
