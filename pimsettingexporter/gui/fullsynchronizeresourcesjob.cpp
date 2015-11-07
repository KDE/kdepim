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


#include "fullsynchronizeresourcesjob.h"

#include <KLocalizedString>

#include <QProgressDialog>
#include <synchronizeresourcejob.h>

FullSynchronizeResourcesJob::FullSynchronizeResourcesJob(QObject *parent)
    : QObject(parent),
      mWindowParent(Q_NULLPTR),
      mProgressDialog(Q_NULLPTR)
{

}

FullSynchronizeResourcesJob::~FullSynchronizeResourcesJob()
{

}

void FullSynchronizeResourcesJob::setResources(const QStringList &lst)
{
    if (lst.isEmpty()) {
        Q_EMIT synchronizeFinished();
        deleteLater();
    } else {
        mResources = lst;
    }
}

void FullSynchronizeResourcesJob::setWindowParent(QWidget *parent)
{
    mWindowParent = parent;
}

void FullSynchronizeResourcesJob::start()
{
    mProgressDialog = new QProgressDialog(QString(), i18n("Cancel"), 0, mResources.count(), mWindowParent);
    mProgressDialog->setWindowTitle(i18n("Synchronize resources"));
    mProgressDialog->setLabelText(i18n("Synchronize resources... It can take some time."));
    mProgressDialog->setWindowModality(Qt::WindowModal);
    //Disable cancel button.
    mProgressDialog->setCancelButton(Q_NULLPTR);

    SynchronizeResourceJob *job = new SynchronizeResourceJob(this);
    //Full synch
    job->setSynchronizeOnlyCollection(false);
    job->setListResources(mResources);
    connect(job, &SynchronizeResourceJob::synchronizationFinished, this, &FullSynchronizeResourcesJob::slotSynchronizeFinished);
    connect(job, &SynchronizeResourceJob::synchronizationInstanceDone, this, &FullSynchronizeResourcesJob::slotSynchronizeInstanceDone);
    connect(job, &SynchronizeResourceJob::synchronizationInstanceFailed, this, &FullSynchronizeResourcesJob::slotSynchronizeInstanceFailed);
    job->start();
}

void FullSynchronizeResourcesJob::slotSynchronizeInstanceDone(const QString &identifier)
{
    Q_EMIT synchronizeInstanceDone(identifier);
    mProgressDialog->setValue(mProgressDialog->value()+1);
}

void FullSynchronizeResourcesJob::slotSynchronizeInstanceFailed(const QString &identifier)
{
    Q_EMIT synchronizeInstanceFailed(identifier);
    mProgressDialog->setValue(mProgressDialog->value()+1);
}

void FullSynchronizeResourcesJob::slotSynchronizeFinished()
{
    mProgressDialog->setValue(mProgressDialog->value()+1);
    Q_EMIT synchronizeFinished();
    deleteLater();
}
