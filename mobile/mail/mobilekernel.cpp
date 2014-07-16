/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "mobilekernel.h"
#include <pimcommon/autocorrection/autocorrection.h>
#include <mailcommon/filter/filteraction.h>
#include <mailcommon/kernel/mailkernel.h>
#include <mailcommon/job/jobscheduler.h>
#include <kpimidentities/identitymanager.h>
#include <messagecomposer/sender/akonadisender.h>

using namespace MailCommon;

static MobileKernel * mySelf = 0;

MobileKernel::MobileKernel() : mMonitor( 0 ), mCollectionModel( 0 ), mMessageSender( 0 ), mConfig( 0 )
{
  CommonKernel; //init

  mJobScheduler = new JobScheduler(0);
  mIdentityManager = new KPIMIdentities::IdentityManager( false, 0, "mIdentityManager" );
  mAutoCorrection = new PimCommon::AutoCorrection();
  mMessageSender = new MessageComposer::AkonadiSender;
  CommonKernel->registerKernelIf( this ); //register KernelIf early, it is used by the Filter classes

  CommonKernel->registerFilterIf( this );

  CommonKernel->registerSettingsIf( this );
}

MobileKernel::~MobileKernel()
{
  delete mJobScheduler;
  delete mIdentityManager;
  delete mMessageSender;
  delete mAutoCorrection;
}

MobileKernel* MobileKernel::self()
{
  if ( !mySelf ) {
    mySelf = new MobileKernel();
  }
  return mySelf;
}

PimCommon::AutoCorrection *MobileKernel::composerAutoCorrection() const
{
  return mAutoCorrection;
}

void MobileKernel::updateSystemTray()
{
//TODO: if it is needed at all
}

void MobileKernel::syncConfig()
{
  mConfig->sync();
}

KSharedConfig::Ptr MobileKernel::config()
{
  if ( !mConfig )
  {
    mConfig = KSharedConfig::openConfig( QLatin1String("kmail-mobilerc") );
  }

  return mConfig;
}

KPIMIdentities::IdentityManager* MobileKernel::identityManager()
{
  return mIdentityManager;
}

MessageComposer::MessageSender* MobileKernel::msgSender()
{
  return mMessageSender;
}


void MobileKernel::setLastSelectedFolder(const Akonadi::Entity::Id& col)
{
 //TODO: implement
  Q_UNUSED( col );
}

Akonadi::Entity::Id MobileKernel::lastSelectedFolder()
{
  return -1; //this was the default on Kmail desktop
}

qreal MobileKernel::closeToQuotaThreshold()
{
  return 80; //this was the default on Kmail desktop
}

bool MobileKernel::excludeImportantMailFromExpiry()
{
  return true;//this was the default on Kmail desktop
}

bool MobileKernel::showPopupAfterDnD()
{
  return false;
}

QStringList MobileKernel::customTemplates()
{
  return QStringList(); //TODO: implement
}

void MobileKernel::openFilterDialog( bool createDummyFilter )
{
  //TODO: Implement filter dialog for mobile
  Q_UNUSED( createDummyFilter );
}

void MobileKernel::createFilter(const QByteArray& field, const QString& value)
{
  //TODO: Implement for mobile (call the dialog with predefined values)
  Q_UNUSED( field );
  Q_UNUSED( value );
}

