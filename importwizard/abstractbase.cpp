/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
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

#include "abstractbase.h"
#include "pimcommon/createresource.h"

#include <KLocale>
#include <KDebug>
#include <KSharedConfig>

#include <akonadi/agenttype.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agentinstancecreatejob.h>

#include <QDBusReply>
#include <QDBusInterface>
#include <QMetaMethod>

using namespace Akonadi;

AbstractBase::AbstractBase()
{
  mCreateResource = new PimCommon::CreateResource();
  connect(mCreateResource,SIGNAL(createResourceInfo(QString)),SLOT(slotCreateResourceInfo(QString)));
  connect(mCreateResource,SIGNAL(createResourceError(QString)),SLOT(slotCreateResourceError(QString)));
}

AbstractBase::~AbstractBase()
{
  delete mCreateResource;
}

QString AbstractBase::createResource(const QString& resources , const QString& name, const QMap<QString, QVariant> &settings)
{
  return mCreateResource->createResource(resources,name,settings);
}

void AbstractBase::slotCreateResourceError(const QString& msg)
{
  addImportError(msg);
}

void AbstractBase::slotCreateResourceInfo(const QString& msg)
{
  addImportInfo(msg);
}

#include "abstractbase.moc"
