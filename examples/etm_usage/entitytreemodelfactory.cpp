/*
    This file is part of Akonadi.

    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

// READ THE README FILE

#include "entitytreemodelfactory.h"

#include <AkonadiCore/collectionfetchjob.h>
#include <AkonadiCore/entitytreemodel.h>
#include <AkonadiCore/changerecorder.h>
#include <AkonadiCore/session.h>

#define WANTED_REMOTE_ID "WANTED_REMOTE_ID"
#include <AkonadiCore/ItemFetchScope>

using namespace Akonadi;

EntityTreeModelFactory::EntityTreeModelFactory(QObject* parent)
  : QObject(parent)
{

}

void EntityTreeModelFactory::createFromRemoteId(const QString& remoteId)
{
  Session *session = new Session("TEST", this);
  CollectionFetchJob *job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::FirstLevel, session);
  job->setProperty(WANTED_REMOTE_ID, remoteId);
  connect(job, SIGNAL(collectionsReceived(Akonadi::Collection::List)), SLOT(collectionsFetched(Akonadi::Collection::List)));
}

void EntityTreeModelFactory::collectionsFetched(const Akonadi::Collection::List& list)
{
  QObject *job = sender();
  if (!job)
    return;
  QString wantedRemoteId = job->property(WANTED_REMOTE_ID).toString();
  if (!wantedRemoteId.isEmpty())
  {
    foreach(const Collection &col, list)
    {
      if (col.remoteId() == wantedRemoteId)
      {
        ChangeRecorder *changeRecorder = new ChangeRecorder(this);
        changeRecorder->setCollectionMonitored(col, true);
        changeRecorder->fetchCollection(true);
        changeRecorder->setAllMonitored(true);
        changeRecorder->itemFetchScope().fetchFullPayload();

        EntityTreeModel *etm = getModel(changeRecorder, parent());

        emit modelCreated(etm);
        return;
      }
    }
    return;
  }
}

EntityTreeModel* EntityTreeModelFactory::getModel(ChangeRecorder *changeRecorder, QObject *parent)
{
  return new EntityTreeModel(changeRecorder, parent);
}

