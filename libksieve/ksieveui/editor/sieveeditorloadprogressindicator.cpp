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

#include "sieveeditorloadprogressindicator.h"

#include <KIconLoader>

#include <QTimer>

using namespace KSieveUi;
SieveEditorLoadProgressIndicator::SieveEditorLoadProgressIndicator(QObject *parent)
    : QObject(parent),
      mProgressCount(0)
{
    mProgressPix = KPixmapSequence(QLatin1String("process-working"), KIconLoader::SizeSmallMedium);
    mProgressTimer = new QTimer(this);
    connect(mProgressTimer, SIGNAL(timeout()), this, SLOT(slotTimerDone()));
}

SieveEditorLoadProgressIndicator::~SieveEditorLoadProgressIndicator()
{
}

void SieveEditorLoadProgressIndicator::startAnimation()
{
    mProgressCount = 0;
    mProgressTimer->start(300);

}

void SieveEditorLoadProgressIndicator::stopAnimation(bool success)
{
    if (mProgressTimer->isActive())
        mProgressTimer->stop();
    Q_EMIT loadFinished(success);
}

void SieveEditorLoadProgressIndicator::slotTimerDone()
{
    Q_EMIT pixmapChanged(mProgressPix.frameAt(mProgressCount));
    ++mProgressCount;
    if (mProgressCount == 8)
        mProgressCount = 0;

    mProgressTimer->start(300);
}

