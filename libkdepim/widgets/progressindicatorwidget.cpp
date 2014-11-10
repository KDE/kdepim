/*
    Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "progressindicatorwidget.h"

#include <QTimer>
#include <KIconLoader>

namespace KPIM
{

IndicatorProgress::IndicatorProgress(ProgressIndicatorWidget *widget, QObject *parent)
    : QObject(parent),
      mProgressCount(0),
      mIndicator(widget),
      mIsActive(false)
{
    mProgressPix =  KIconLoader::global()->loadPixmapSequence(QLatin1String("process-working"), KIconLoader::SizeSmallMedium);
    mProgressTimer = new QTimer(this);
    connect(mProgressTimer, &QTimer::timeout, this, &IndicatorProgress::slotTimerDone);
}

IndicatorProgress::~IndicatorProgress()
{
}

void IndicatorProgress::slotTimerDone()
{
    mIndicator->setPixmap(mProgressPix.frameAt(mProgressCount));
    ++mProgressCount;
    if (mProgressCount == 8) {
        mProgressCount = 0;
    }

    mProgressTimer->start(300);
}

void IndicatorProgress::startAnimation()
{
    mProgressCount = 0;
    mProgressTimer->start(300);
    mIsActive = true;
}

void IndicatorProgress::stopAnimation()
{
    mIsActive = false;
    if (mProgressTimer->isActive()) {
        mProgressTimer->stop();
    }
    mIndicator->clear();
}

bool IndicatorProgress::isActive() const
{
    return mIsActive;
}

class ProgressIndicatorWidgetPrivate
{
public:
    ProgressIndicatorWidgetPrivate(ProgressIndicatorWidget *qq)
        : q(qq)
    {
        indicator = new IndicatorProgress(q);
    }

    ~ProgressIndicatorWidgetPrivate()
    {
        delete indicator;
    }

    IndicatorProgress *indicator;
    ProgressIndicatorWidget *q;
};

ProgressIndicatorWidget::ProgressIndicatorWidget(QWidget *parent)
    : QLabel(parent),
      d(new ProgressIndicatorWidgetPrivate(this))
{
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
}

ProgressIndicatorWidget::~ProgressIndicatorWidget()
{
    delete d;
}

void ProgressIndicatorWidget::start()
{
    d->indicator->startAnimation();
}

void ProgressIndicatorWidget::stop()
{
    d->indicator->stopAnimation();
}

bool ProgressIndicatorWidget::isActive() const
{
    return d->indicator->isActive();
}

}

