/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "composerimageresizewidget.h"
#include "composerimageresizetooltip.h"

#include <QMouseEvent>
#include <QPainter>

namespace ComposerEditorNG
{

static const int resizeSquareSize = 7;

class ComposerImageResizeWidgetPrivate
{
public:
    enum ResizeDirection {
        None,
        Top,
        Bottom,
        Left,
        Right,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    ComposerImageResizeWidgetPrivate(ComposerImageResizeWidget *qq, const QWebElement& element)
        : q(qq),
          imageSizetoolTip(0),
          imageElement(element),
          direction(None),
          mousePressed(false)
    {
        q->resize(imageElement.geometry().size());
    }

    void resizeElement(const QPoint& pos);
    QSize resizeImage(const QPoint& pos);
    void setResizeDirectionCursor(const QPoint& pos);
    ResizeDirection resizeDirection(const QPoint& pos);

    ComposerImageResizeWidget *q;
    ComposerImageResizeToolTip *imageSizetoolTip;
    QWebElement imageElement;
    ResizeDirection direction;
    QPoint firstPosition;
    bool mousePressed;
};

void ComposerImageResizeWidgetPrivate::setResizeDirectionCursor(const QPoint& pos)
{
    ResizeDirection dir = resizeDirection(pos);
    switch(dir) {
    case None:
        q->setCursor(Qt::ArrowCursor);
        break;
    case Top:
    case Bottom:
        q->setCursor(Qt::SizeVerCursor);
        break;
    case Left:
    case Right:
        q->setCursor(Qt::SizeHorCursor);
        break;
    case TopLeft:
    case BottomRight:
        q->setCursor(Qt::SizeFDiagCursor);
        break;
    case TopRight:
    case BottomLeft:
        q->setCursor(Qt::SizeBDiagCursor);
        break;
    }
}

ComposerImageResizeWidgetPrivate::ResizeDirection ComposerImageResizeWidgetPrivate::resizeDirection(const QPoint& pos)
{
    ResizeDirection dir;
    const QRect r(imageElement.geometry());
    if (QRect(0,0,resizeSquareSize,resizeSquareSize).contains(pos)) {
        dir = TopLeft;
    } else if (QRect(0,r.height()-resizeSquareSize,resizeSquareSize,resizeSquareSize).contains(pos)) {
        dir = BottomLeft;
    } else if (QRect(r.width()-resizeSquareSize,r.height()-resizeSquareSize,resizeSquareSize,resizeSquareSize).contains(pos)) {
        dir = BottomRight;
    } else if (QRect(r.width()-resizeSquareSize,0,resizeSquareSize,resizeSquareSize).contains(pos)) {
        dir = TopRight;
    } else if (QRect(0,0,r.width(),resizeSquareSize).contains(pos)) {
        dir = Top;
    } else if (QRect(0,r.height()-resizeSquareSize,r.width(),resizeSquareSize).contains(pos)) {
        dir = Bottom;
    } else if (QRect(0,0,resizeSquareSize,r.height()).contains(pos)) {
        dir = Left;
    } else if (QRect(r.width()-resizeSquareSize,0,resizeSquareSize,r.height()).contains(pos)) {
        dir = Right;
    } else if (QRect(r.width(),resizeSquareSize,resizeSquareSize,resizeSquareSize).contains(pos)) {
        dir = TopLeft;
    } else {
        dir = None;
    }
    return dir;
}

void ComposerImageResizeWidgetPrivate::resizeElement(const QPoint& pos)
{
    const QSize size = resizeImage(pos);
    if (size.width() != -1) {
        imageElement.setAttribute(QLatin1String("width"),QString::number(size.width()));
    }
    if (size.height() != -1) {
        imageElement.setAttribute(QLatin1String("height"),QString::number(size.height()));
    }
    q->resize(size);
}

QSize ComposerImageResizeWidgetPrivate::resizeImage(const QPoint& pos)
{
    int width = -1;
    int height = -1;
    switch(direction) {
    case None:
        break;
    case Top:
        height = imageElement.attribute(QLatin1String("height")).toInt() - pos.y() - firstPosition.y();
        break;
    case Bottom:
        height = imageElement.attribute(QLatin1String("height")).toInt() + pos.y() - firstPosition.y();
        break;
    case Left:
        width = imageElement.attribute(QLatin1String("width")).toInt() - pos.x() - firstPosition.x();
        break;
    case Right:
        width = imageElement.attribute(QLatin1String("width")).toInt() + pos.x() - firstPosition.x();
        break;
    case TopLeft:
        width = imageElement.attribute(QLatin1String("width")).toInt() - pos.x() - firstPosition.x();
        height = imageElement.attribute(QLatin1String("height")).toInt() - pos.y() - firstPosition.y();
        break;
    case BottomRight:
        width = imageElement.attribute(QLatin1String("width")).toInt() + pos.x() - firstPosition.x();
        height = imageElement.attribute(QLatin1String("height")).toInt() + pos.y() - firstPosition.y();
        break;
    case TopRight:
        height = imageElement.attribute(QLatin1String("height")).toInt() - pos.y() - firstPosition.y();
        width = imageElement.attribute(QLatin1String("width")).toInt() + pos.x() - firstPosition.x();
        break;
    case BottomLeft:
        height = imageElement.attribute(QLatin1String("height")).toInt() + pos.y() - firstPosition.y();
        width = imageElement.attribute(QLatin1String("width")).toInt() - pos.x() - firstPosition.x();
        break;
    }
    return QSize(width, height);
}

ComposerImageResizeWidget::ComposerImageResizeWidget(const QWebElement &element, QWidget *parent)
    : QWidget(parent), d(new ComposerImageResizeWidgetPrivate(this,element))
{
    setMouseTracking(true);
}

ComposerImageResizeWidget::~ComposerImageResizeWidget()
{
    delete d;
}

void ComposerImageResizeWidget::mouseMoveEvent( QMouseEvent * event )
{
    if (!d->mousePressed) {
        d->setResizeDirectionCursor(event->pos());
    } else if (d->direction!=ComposerImageResizeWidgetPrivate::None){
        const QSize size = d->resizeImage(event->pos());
        if (!d->imageSizetoolTip) {
            d->imageSizetoolTip = new ComposerImageResizeToolTip(this);
        }
        d->imageSizetoolTip->show();
        d->imageSizetoolTip->displaySize(size);
        d->imageSizetoolTip->move(QCursor::pos());
        //resize(d->resizeImage(event->pos()));
        //TODO resize
    }
}

void ComposerImageResizeWidget::mousePressEvent( QMouseEvent * event )
{
    d->direction = d->resizeDirection(event->pos());
    if (d->direction!=ComposerImageResizeWidgetPrivate::None) {
        d->mousePressed = true;
        d->firstPosition = event->pos();
    } else {
        event->ignore();
    }

}

void ComposerImageResizeWidget::mouseReleaseEvent( QMouseEvent * event )
{
    if (d->mousePressed) {
        d->resizeElement(event->pos());
        d->mousePressed = false;
        d->direction = ComposerImageResizeWidgetPrivate::None;
        if (d->imageSizetoolTip) {
            d->imageSizetoolTip->hide();
        }
    }
}

void ComposerImageResizeWidget::paintEvent( QPaintEvent * )
{
    if (d->imageElement.isNull())
        return;

    //TODO fix when we scroll area
    const int width = d->imageElement.geometry().width();
    const int height = d->imageElement.geometry().height();
    QPainter painter(this);

    painter.drawRect(QRect(0,0,width,height));
    painter.setPen(Qt::white);
    painter.drawRect(QRect(0,0,resizeSquareSize,resizeSquareSize));
    painter.drawRect(QRect(width-resizeSquareSize,0,resizeSquareSize,resizeSquareSize));
    painter.drawRect(QRect(0,height-resizeSquareSize,resizeSquareSize,resizeSquareSize));
    painter.drawRect(QRect(width-resizeSquareSize,height-resizeSquareSize,resizeSquareSize,resizeSquareSize));

    painter.drawRect(QRect((width-resizeSquareSize)/2,0,resizeSquareSize,resizeSquareSize));
    painter.drawRect(QRect((width-resizeSquareSize)/2,height-resizeSquareSize,resizeSquareSize,resizeSquareSize));

    painter.drawRect(QRect(0,(height-resizeSquareSize)/2,resizeSquareSize,resizeSquareSize));
    painter.drawRect(QRect(width-resizeSquareSize,(height-resizeSquareSize)/2,resizeSquareSize,resizeSquareSize));
}


}

