/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>
  based on code:
Copyright 2009 Aurélien Gâteau <agateau@kde.org>
Copyright 2009 Kåre Sårs <kare.sars@iki.fi>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "splittercollapser.h"

// Qt
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QSplitter>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QTimeLine>

using namespace PimCommon;

enum Direction {
    LeftToRight,
    RightToLeft,
    TopToBottom,
    BottomToTop
};

const static int TIMELINE_DURATION = 500;

const static qreal MINIMUM_OPACITY = 0.3;

class ArrowTypes {
public:
    ArrowTypes()
        : mVisible(Qt::NoArrow),
          mNotVisible(Qt::NoArrow)
    {
    }

    ArrowTypes(Qt::ArrowType t1, Qt::ArrowType t2)
        : mVisible(t1),
          mNotVisible(t2)
    {

    }

    Qt::ArrowType arrowType(bool isVisible) const
    {
        return isVisible ? mVisible : mNotVisible;
    }
private:
    Qt::ArrowType mVisible;
    Qt::ArrowType mNotVisible;
};

class SplitterCollapser::Private
{
public:
    Private(SplitterCollapser *qq);

    SplitterCollapser *q;
    QSplitter *mSplitter;
    QWidget *mWidget;
    Direction mDirection;
    QTimeLine *mOpacityTimeLine;
    QList<int> mSizeAtCollapse;

    bool isVertical() const;

    bool isVisible() const;

    void updatePosition();

    void updateArrow();

    void widgetEventFilter(QEvent *event);

    void updateOpacity();

    void startTimeLine();
};

SplitterCollapser::Private::Private(PimCommon::SplitterCollapser *qq)
    : q(qq),
      mSplitter(0),
      mWidget(0),
      mOpacityTimeLine(0)
{

}

bool SplitterCollapser::Private::isVertical() const
{
    return (mSplitter->orientation() == Qt::Vertical);
}

bool SplitterCollapser::Private::isVisible() const
{
    bool isVisible = mWidget->isVisible();
    const QRect widgetRect = mWidget->geometry();
    if (isVisible) {
        const QPoint br = widgetRect.bottomRight();
        if ((br.x() <= 0) || (br.y() <= 0)) {
            isVisible = false;
        }
    }
    return isVisible;
}

void SplitterCollapser::Private::updatePosition()
{
    int x = 0;
    int y = 0;
    const QRect widgetRect = mWidget->geometry();
    const int handleWidth = mSplitter->handleWidth();

    if (!isVertical()) {
        const int splitterWidth = mSplitter->width();
        const int width = q->width();
        // FIXME: Make this configurable
        y = 30;
        if (mDirection == LeftToRight) {
            if (isVisible()) {
                x = widgetRect.right() + handleWidth;
            } else {
                x = 0;
            }
        } else { // RightToLeft
            if (isVisible()) {
                x = widgetRect.left() - handleWidth - width;
            } else {
                x = splitterWidth - handleWidth - width;
            }
        }
    } else {
        x = 30;
        const int height = q->height();
        const int splitterHeight = mSplitter->height();
        if (mDirection == TopToBottom) {
            if (isVisible()) {
                y = widgetRect.bottom() - height;
            } else {
                y = 0;
            }
        } else { // BottomToTop
            if (isVisible()) {
                y = widgetRect.top() + handleWidth;
            } else {
                y = splitterHeight - handleWidth - height;
            }
        }
    }
    q->move(x, y);
}

void SplitterCollapser::Private::updateArrow()
{
    static QHash<Direction, ArrowTypes> arrowForDirection;
    if (arrowForDirection.isEmpty()) {
        arrowForDirection[LeftToRight] = ArrowTypes(Qt::LeftArrow,  Qt::RightArrow);
        arrowForDirection[RightToLeft] = ArrowTypes(Qt::RightArrow, Qt::LeftArrow);
        arrowForDirection[TopToBottom] = ArrowTypes(Qt::UpArrow,    Qt::DownArrow);
        arrowForDirection[BottomToTop] = ArrowTypes(Qt::DownArrow,  Qt::UpArrow);
    }
    q->setArrowType(arrowForDirection[mDirection].arrowType(isVisible()));
}

void SplitterCollapser::Private::widgetEventFilter(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Resize:
        updatePosition();
        updateOpacity();
        break;

    case QEvent::Move:
    case QEvent::Show:
    case QEvent::Hide:
        updatePosition();
        updateOpacity();
        updateArrow();
        break;

    default:
        break;
    }
}

void SplitterCollapser::Private::updateOpacity()
{
    const QPoint pos = q->parentWidget()->mapFromGlobal(QCursor::pos());
    const QRect opaqueRect = q->geometry();
    const bool opaqueCollapser = opaqueRect.contains(pos);
    const int frame = mOpacityTimeLine->currentFrame();
    if (opaqueCollapser && frame == mOpacityTimeLine->startFrame()) {
        mOpacityTimeLine->setDirection(QTimeLine::Forward);
        startTimeLine();
    } else if (!opaqueCollapser && frame == mOpacityTimeLine->endFrame()) {
        mOpacityTimeLine->setDirection(QTimeLine::Backward);
        startTimeLine();
    }
}

void SplitterCollapser::Private::startTimeLine()
{
    if (mOpacityTimeLine->state() != QTimeLine::Running) {
        mOpacityTimeLine->start();
    }
}


SplitterCollapser::SplitterCollapser(QSplitter *splitter, QWidget *widget, QWidget *parent)
    : QToolButton(parent),
      d(new Private(this))
{
    setObjectName(QLatin1String("splittercollapser"));
    // We do not want our collapser to be added as a regular widget in the
    // splitter!
    setAttribute(Qt::WA_NoChildEventsForParent);

    d->mOpacityTimeLine = new QTimeLine(TIMELINE_DURATION, this);
    d->mOpacityTimeLine->setFrameRange(int(MINIMUM_OPACITY * 1000), 1000);
    connect(d->mOpacityTimeLine, SIGNAL(valueChanged(qreal)), SLOT(update()));

    d->mWidget = widget;
    d->mWidget->installEventFilter(this);

    qApp->installEventFilter(this);

    d->mSplitter = splitter;
    setParent(d->mSplitter);

    switch(splitter->orientation()) {
    case Qt::Horizontal: {
        if (splitter->indexOf(widget) < splitter->count() / 2) {
            d->mDirection = LeftToRight;
        } else {
            d->mDirection = RightToLeft;
        }
        break;
    }
    case Qt::Vertical:
        if (splitter->indexOf(widget) < splitter->count() / 2) {
            d->mDirection = TopToBottom;
        } else {
            d->mDirection = BottomToTop;
        }
        break;
    }

    connect(this, SIGNAL(clicked()), SLOT(slotClicked()));

    show();
}

SplitterCollapser::~SplitterCollapser()
{
    delete d;
}

bool SplitterCollapser::isCollapsed() const
{
    return !d->isVisible();
}

bool SplitterCollapser::eventFilter(QObject *object, QEvent *event)
{
    if (object == d->mWidget) {
        d->widgetEventFilter(event);
    } else { /* app */
        if (event->type() == QEvent::MouseMove) {
            d->updateOpacity();
        }
    }
    return false;
}

QSize SplitterCollapser::sizeHint() const
{
    const int extent = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    QSize sh(extent * 3 / 4, extent * 240 / 100);
    if (d->isVertical()) {
        sh.transpose();
    }
    return sh;
}

void SplitterCollapser::slotClicked()
{
    QList<int> sizes = d->mSplitter->sizes();
    const int index = d->mSplitter->indexOf(d->mWidget);
    if (d->isVisible()) {
        d->mSizeAtCollapse = sizes;
        sizes[index] = 0;
    } else {
        if (!d->mSizeAtCollapse.isEmpty()) {
            sizes = d->mSizeAtCollapse;
        } else {
            if (d->isVertical()) {
                sizes[index] = d->mWidget->sizeHint().height();
            } else {
                sizes[index] = d->mWidget->sizeHint().width();
            }
        }
    }
    d->mSplitter->setSizes(sizes);
}

void SplitterCollapser::slotCollapse()
{
    if (d->isVisible()) {
        slotClicked();
    }
    // else do nothing
}

void SplitterCollapser::slotRestore()
{
    if (!d->isVisible()) {
        slotClicked();
    }
    // else do nothing
}

void SplitterCollapser::slotSetCollapsed(bool collapse)
{
    if (collapse == d->isVisible()) {
        slotClicked();
    }
    // else do nothing
}

void SplitterCollapser::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);
    const qreal opacity = d->mOpacityTimeLine->currentFrame() / 1000.;
    painter.setOpacity(opacity);

    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    if (d->mDirection == LeftToRight) {
        opt.rect.setLeft(-width());
    } else {
        opt.rect.setWidth(width() * 2);
    }
    painter.drawPrimitive(QStyle::PE_PanelButtonTool, opt);

    QStyleOptionToolButton opt2;
    initStyleOption(&opt2);
    painter.drawControl(QStyle::CE_ToolButtonLabel, opt2);
}



