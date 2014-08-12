/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include <AkonadiWidgets/standardactionmanager.h>
#include <kaction.h>
#include <klocale.h>

using namespace Akonadi;

template <class T>
void ActionHelper::adaptStandardActionTexts( T *manager )
{
  manager->setActionText( StandardActionManager::SynchronizeResources, ki18np( "Synchronize This Account", "Synchronize These Accounts" ) );
  manager->action( StandardActionManager::SynchronizeResources )->setText( i18n("Synchronize This Account") );
  manager->action( StandardActionManager::ManageLocalSubscriptions )->setText( i18n( "Local Subscriptions" ) );
  manager->action( StandardActionManager::ResourceProperties )->setText( i18n( "Account Properties" ) );
  manager->action( StandardActionManager::ToggleWorkOffline )->setText( i18n( "Work Offline" ) );
}
