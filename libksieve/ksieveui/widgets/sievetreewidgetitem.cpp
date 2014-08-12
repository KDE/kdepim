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

#include "sievetreewidgetitem.h"

#include <KIconLoader>

#include <QTimer>

using namespace KSieveUi;
SieveTreeWidgetProgress::SieveTreeWidgetProgress(SieveTreeWidgetItem *item, QObject *parent)
    : QObject(parent),
      mProgressCount(0),
      mItem(item)
{
    KIconLoader loader;
    mProgressPix = loader.loadPixmapSequence(QLatin1String("process-working"), KIconLoader::SizeSmallMedium);
    mProgressTimer = new QTimer(this);
    connect(mProgressTimer, SIGNAL(timeout()), this, SLOT(slotTimerDone()));
}

SieveTreeWidgetProgress::~SieveTreeWidgetProgress()
{
}

void SieveTreeWidgetProgress::slotTimerDone()
{
    mItem->setProgressAnimation(mProgressPix.frameAt(mProgressCount));
    ++mProgressCount;
    if (mProgressCount == 8)
        mProgressCount = 0;

    mProgressTimer->start(300);
}

void SieveTreeWidgetProgress::startAnimation()
{
    mProgressCount = 0;
    mProgressTimer->start(300);
}

void SieveTreeWidgetProgress::stopAnimation()
{
    if (mProgressTimer->isActive())
        mProgressTimer->stop();
    mItem->setDefaultIcon();
}

SieveTreeWidgetItem::SieveTreeWidgetItem(QTreeWidget *treeWidget, QTreeWidgetItem *item)
    : QTreeWidgetItem(treeWidget, item)
{
    mProgress = new SieveTreeWidgetProgress(this);
}

SieveTreeWidgetItem::~SieveTreeWidgetItem()
{
    delete mProgress;
}

void SieveTreeWidgetItem::startAnimation()
{
    mProgress->startAnimation();
}

void SieveTreeWidgetItem::stopAnimation()
{
    mProgress->stopAnimation();
}

void SieveTreeWidgetItem::setProgressAnimation(const QPixmap& pix)
{
    setIcon(0, QIcon(pix));
}

void SieveTreeWidgetItem::setDefaultIcon()
{
    setIcon( 0, SmallIcon( QLatin1String("network-server") ) );
}

