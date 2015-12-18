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

#include "kaddressbookplugininterface.h"
#include "pimcommon/genericpluginmanager.h"
#include "kaddressbook_debug.h"
#include "mainwidget.h"

#include <KActionCollection>

#include <pimcommon/genericplugin.h>

KAddressBookPluginInterface::KAddressBookPluginInterface(KActionCollection *ac, QObject *parent)
    : PimCommon::PluginInterface(ac, parent),
      mMainWidget(Q_NULLPTR)
{
    setPluginName(QStringLiteral("kaddressbook"));
    setServiceTypeName(QStringLiteral("KAddressBook/MainViewPlugin"));
    initializePlugins();
}

KAddressBookPluginInterface::~KAddressBookPluginInterface()
{

}

void KAddressBookPluginInterface::initializeInterfaceRequires(PimCommon::GenericPluginInterface *interface)
{
    PimCommon::GenericPluginInterface::RequireTypes requires = interface->requires();
    if (requires & PimCommon::GenericPluginInterface::CurrentItems) {
        interface->setCurrentItems(mMainWidget->collectSelectedAllContactsItem());
    }
    if (requires & PimCommon::GenericPluginInterface::Items) {
        interface->setItems(mMainWidget->selectedItems());
        qCDebug(KADDRESSBOOK_LOG) << "PimCommon::GenericPluginInterface::Items not implemented";
    }
    if (requires & PimCommon::GenericPluginInterface::CurrentCollection) {
        interface->setCurrentCollection(mMainWidget->currentAddressBook());
    }
    if (requires & PimCommon::GenericPluginInterface::Collections) {
        qCDebug(KADDRESSBOOK_LOG) << "PimCommon::GenericPluginInterface::Collections not implemented";
        //TODO
    }
}

void KAddressBookPluginInterface::setMainWidget(MainWidget *mainWidget)
{
    mMainWidget = mainWidget;
}

