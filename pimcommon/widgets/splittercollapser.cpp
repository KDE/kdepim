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
#include <QDebug>

using namespace PimCommon;

enum Direction {
    LeftToRight =0,
    RightToLeft,
    TopToBottom,
    BottomToTop
};

const static int TIMELINE_DURATION = 500;

const static qreal MINIMUM_OPACITY = 0.3;

static const struct {
    Qt::ArrowType arrowVisible;
    Qt::ArrowType notArrowVisible;
} s_arrowDirection[] = {
    {Qt::LeftArrow,  Qt::RightArrow},
    {Qt::RightArrow, Qt::LeftArrow},
    {Qt::UpArrow, Qt::DownArrow},
    {Qt::DownArrow,  Qt::UpArrow}
};

class SplitterCollapser::Private
{
public:
    Private(SplitterCollapser *qq);

    SplitterCollapser *q;
    QSplitter *splitter;
    QWidget *childWidget;
    Direction direction;
    QTimeLine *opacityTimeLine;
    QList<int> sizeAtCollapse;

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
      splitter(0),
      childWidget(0),
      opacityTimeLine(0)
{

}

bool SplitterCollapser::Private::isVertical() const
{
    return (splitter->orientation() == Qt::Vertical);
}

bool SplitterCollapser::Private::isVisible() const
{
    const QRect widgetRect = childWidget->geometry();
    const QPoint br = widgetRect.bottomRight();
    if ((br.x() <= 0) || (br.y() <= 0)) {
        return false;
    } else {
        return true;
    }
}

void SplitterCollapser::Private::updatePosition()
{
    int x = 0;
    int y = 0;
    const QRect widgetRect = childWidget->geometry();
    const int handleWidth = splitter->handleWidth();

    if (!isVertical()) {
        const int splitterWidth = splitter->width();
        const int width = q->width();
        // FIXME: Make this configurable
        y = 30;
        if (direction == LeftToRight) {
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
        const int splitterHeight = splitter->height();
        if (direction == TopToBottom) {
            if (isVisible()) {
                y = widgetRect.bottom() + handleWidth;
            } else {
                y = 0;
            }
        } else { // BottomToTop
            if (isVisible()) {
                y = widgetRect.top() - handleWidth - height;
            } else {
                y = splitterHeight - handleWidth - height;
            }
        }
    }
    q->move(x, y);
}

void SplitterCollapser::Private::updateArrow()
{
    q->setArrowType(isVisible() ? s_arrowDirection[direction].arrowVisible : s_arrowDirection[direction].notArrowVisible);
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
    const int frame = opacityTimeLine->currentFrame();
    if (opaqueCollapser && frame == opacityTimeLine->startFrame()) {
        opacityTimeLine->setDirection(QTimeLine::Forward);
        startTimeLine();
    } else if (!opaqueCollapser && frame == opacityTimeLine->endFrame()) {
        opacityTimeLine->setDirection(QTimeLine::Backward);
        startTimeLine();
    }
}

void SplitterCollapser::Private::startTimeLine()
{
    if (opacityTimeLine->state() != QTimeLine::Running) {
        opacityTimeLine->start();
    }
}


SplitterCollapser::SplitterCollapser(QWidget *childWidget, QSplitter *splitter)
    : QToolButton(),
      d(new Private(this))
{
    setObjectName(QLatin1String("splittercollapser"));
    // We do not want our collapser to be added as a regular widget in the
    // splitter!
    setAttribute(Qt::WA_NoChildEventsForParent);

    d->opacityTimeLine = new QTimeLine(TIMELINE_DURATION, this);
    d->opacityTimeLine->setFrameRange(int(MINIMUM_OPACITY * 1000), 1000);
    connect(d->opacityTimeLine, SIGNAL(valueChanged(qreal)), SLOT(update()));

    d->childWidget = childWidget;
    d->childWidget->installEventFilter(this);

    qApp->installEventFilter(this);

    d->splitter = splitter;
    setParent(d->splitter);

    switch(splitter->orientation()) {
    case Qt::Horizontal: {
        if (splitter->indexOf(childWidget) < splitter->count() / 2) {
            d->direction = LeftToRight;
        } else {
            d->direction = RightToLeft;
        }
        break;
    }
    case Qt::Vertical:
        if (splitter->indexOf(childWidget) < splitter->count() / 2) {
            d->direction = TopToBottom;
        } else {
            d->direction = BottomToTop;
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
    if (object == d->childWidget) {
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
    QList<int> sizes = d->splitter->sizes();
    const int index = d->splitter->indexOf(d->childWidget);
    if (d->isVisible()) {
        d->sizeAtCollapse = sizes;
        sizes[index] = 0;
    } else {
        if (!d->sizeAtCollapse.isEmpty()) {
            sizes = d->sizeAtCollapse;
        } else {
            if (d->isVertical()) {
                sizes[index] = d->childWidget->sizeHint().height();
            } else {
                sizes[index] = d->childWidget->sizeHint().width();
            }
        }
    }
    d->splitter->setSizes(sizes);
}

void SplitterCollapser::collapse()
{
    if (d->isVisible()) {
        slotClicked();
    }
    // else do nothing
}

void SplitterCollapser::restore()
{
    if (!d->isVisible()) {
        slotClicked();
    }
    // else do nothing
}

void SplitterCollapser::setCollapsed(bool collapse)
{
    if (collapse == d->isVisible()) {
        slotClicked();
    }
    // else do nothing
}

void SplitterCollapser::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);
    const qreal opacity = d->opacityTimeLine->currentFrame() / 1000.;
    painter.setOpacity(opacity);

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    if (d->isVertical()) {
        if (d->direction == TopToBottom) {
            opt.rect.setTop(-height());
        } else {
            opt.rect.setHeight(height()*2);
        }
    } else {
        if (d->direction == LeftToRight) {
            opt.rect.setLeft(-width());
        } else {
            opt.rect.setWidth(width() * 2);
        }
    }
    painter.drawPrimitive(QStyle::PE_PanelButtonTool, opt);

    QStyleOptionToolButton opt2;
    initStyleOption(&opt2);
    painter.drawControl(QStyle::CE_ToolButtonLabel, opt2);
}



