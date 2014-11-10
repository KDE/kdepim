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

#ifndef PROGRESSINDICATORWIDGET_H
#define PROGRESSINDICATORWIDGET_H

#include "kdepim_export.h"

#include <KPixmapSequence>

#include <QLabel>

class QTimer;
namespace KPIM
{
class ProgressIndicatorWidgetPrivate;
class ProgressIndicatorWidget;
class IndicatorProgress : public QObject
{
    Q_OBJECT
public:
    explicit IndicatorProgress(ProgressIndicatorWidget *widget, QObject *parent = 0);
    ~IndicatorProgress();

    bool isActive() const;

    void startAnimation();
    void stopAnimation();

private Q_SLOTS:
    void slotTimerDone();

private:
    int mProgressCount;
    KPixmapSequence mProgressPix;
    QTimer *mProgressTimer;
    ProgressIndicatorWidget *mIndicator;
    bool mIsActive;
};

class KDEPIM_EXPORT ProgressIndicatorWidget : public QLabel
{
    Q_OBJECT
public:
    explicit ProgressIndicatorWidget(QWidget *parent = 0);
    ~ProgressIndicatorWidget();

public:
    /**
     * @since 4.12
     */
    bool isActive() const;

public Q_SLOTS:
    void start();
    void stop();

private:
    friend class ProgressIndicatorWidgetPrivate;
    ProgressIndicatorWidgetPrivate *const d;
};

}

#endif // PROGRESSINDICATORWIDGET_H
