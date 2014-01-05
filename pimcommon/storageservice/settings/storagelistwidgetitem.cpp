/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include "storagelistwidgetitem.h"

#include <KIconLoader>
#include <QTimer>

using namespace PimCommon;


StorageListWidgetItemProgress::StorageListWidgetItemProgress(StorageListWidgetItem *item, QObject *parent)
    : QObject(parent),
      mProgressCount(0),
      mItem(item)
{
    mProgressPix = KPixmapSequence(QLatin1String("process-working"), KIconLoader::SizeSmallMedium);
    mProgressTimer = new QTimer(this);
    connect(mProgressTimer, SIGNAL(timeout()), this, SLOT(slotTimerDone()));
}

StorageListWidgetItemProgress::~StorageListWidgetItemProgress()
{
}

void StorageListWidgetItemProgress::slotTimerDone()
{
    mItem->setProgressAnimation(mProgressPix.frameAt(mProgressCount));
    ++mProgressCount;
    if (mProgressCount == 8)
        mProgressCount = 0;

    mProgressTimer->start(300);
}

void StorageListWidgetItemProgress::startAnimation()
{
    mProgressCount = 0;
    mProgressTimer->start(300);
}

void StorageListWidgetItemProgress::stopAnimation()
{
    if (mProgressTimer->isActive())
        mProgressTimer->stop();
    mItem->resetToDefaultIcon();
}


StorageListWidgetItem::StorageListWidgetItem(QListWidget *parent)
    : QListWidgetItem(parent)
{
    mProgress = new StorageListWidgetItemProgress(this);
}

StorageListWidgetItem::~StorageListWidgetItem()
{
    delete mProgress;
}

void StorageListWidgetItem::startAnimation()
{
    mProgress->startAnimation();
}

void StorageListWidgetItem::stopAnimation()
{
    mProgress->stopAnimation();
}

void StorageListWidgetItem::setProgressAnimation(const QPixmap &pix)
{
    setIcon(QIcon(pix));
}

void StorageListWidgetItem::setDefaultIcon(const QString &defaultIconName)
{
    mDefaultIcon = KIcon(defaultIconName);
}

void StorageListWidgetItem::resetToDefaultIcon()
{
    setIcon(mDefaultIcon);
}
