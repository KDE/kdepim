/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

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
          imageElement(element),
          direction(None),
          mousePressed(false)
    {
        q->resize(imageElement.geometry().size());
    }

    void resizeImage(const QPoint& pos);
    void setResizeDirectionCursor(const QPoint& pos);
    ResizeDirection resizeDirection(const QPoint& pos);

    ComposerImageResizeWidget *q;
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
    if(QRect(0,0,resizeSquareSize,resizeSquareSize).contains(pos)) {
        dir = TopLeft;
    } else if(QRect(0,r.height()-resizeSquareSize,resizeSquareSize,resizeSquareSize).contains(pos)) {
        dir = BottomLeft;
    } else if(QRect(r.width()-resizeSquareSize,r.height()-resizeSquareSize,resizeSquareSize,resizeSquareSize).contains(pos)) {
        dir = BottomRight;
    } else if(QRect(r.width()-resizeSquareSize,0,resizeSquareSize,resizeSquareSize).contains(pos)) {
        dir = TopRight;
    } else if(QRect(0,0,r.width(),resizeSquareSize).contains(pos)) {
        dir = Top;
    } else if(QRect(0,r.height()-resizeSquareSize,r.width(),resizeSquareSize).contains(pos)) {
        dir = Bottom;
    } else if(QRect(0,0,resizeSquareSize,r.height()).contains(pos)) {
        dir = Left;
    } else if(QRect(r.width()-resizeSquareSize,0,resizeSquareSize,r.height()).contains(pos)) {
        dir = Right;
    } else if(QRect(r.width(),resizeSquareSize,resizeSquareSize,resizeSquareSize).contains(pos)) {
        dir = TopLeft;
    } else {
        dir = None;
    }
    return dir;
}

void ComposerImageResizeWidgetPrivate::resizeImage(const QPoint& pos)
{
    int width = -1;
    int height = -1;
    qDebug()<<" imageElement.geometry().size()"<<imageElement.geometry().size();
    switch(direction) {
    case None:
        break;
    case Top:
        break;
    case Bottom:
        height = imageElement.attribute(QLatin1String("height")).toInt() + pos.y() - firstPosition.y();
        break;
    case Left:
        break;
    case Right:
        break;
    case TopLeft:
        break;
    case BottomRight:
        break;
    case TopRight:
        break;
    case BottomLeft:
        break;
    }
    if(width != -1) {
        imageElement.setAttribute(QLatin1String("width"),QString::number(width));
    }
    if(height != -1) {
        imageElement.setAttribute(QLatin1String("height"),QString::number(height));
    }
    q->resize(imageElement.geometry().size());
    qDebug()<<" AFTER :"<<imageElement.geometry().size();
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
    if(!d->mousePressed) {
        d->setResizeDirectionCursor(event->pos());
    } else if(d->direction!=ComposerImageResizeWidgetPrivate::None){
        //TODO resize
    }
}

void ComposerImageResizeWidget::mousePressEvent( QMouseEvent * event )
{
    d->direction = d->resizeDirection(event->pos());
    if(d->direction!=ComposerImageResizeWidgetPrivate::None) {
        d->mousePressed = true;
        d->firstPosition = event->pos();
    } else {
        event->ignore();
    }

}

void ComposerImageResizeWidget::mouseReleaseEvent( QMouseEvent * event )
{
    if(d->mousePressed) {
        d->resizeImage(event->pos());
        d->mousePressed = false;
        d->direction = ComposerImageResizeWidgetPrivate::None;
    }
}

void ComposerImageResizeWidget::paintEvent( QPaintEvent * )
{
    if(d->imageElement.isNull())
        return;


    const int width = d->imageElement.geometry().width();
    const int height = d->imageElement.geometry().height();
    QPainter painter(this);

    painter.drawRect(QRect(0,0,width,height));
    painter.fillRect(QRect(0,0,resizeSquareSize,resizeSquareSize),Qt::white);
    painter.fillRect(QRect(width-resizeSquareSize,0,resizeSquareSize,resizeSquareSize),Qt::white);
    painter.fillRect(QRect(0,height-resizeSquareSize,resizeSquareSize,resizeSquareSize),Qt::white);
    painter.fillRect(QRect(width-resizeSquareSize,height-resizeSquareSize,resizeSquareSize,resizeSquareSize),Qt::white);

    painter.fillRect(QRect((width-resizeSquareSize)/2,0,resizeSquareSize,resizeSquareSize),Qt::white);
    painter.fillRect(QRect((width-resizeSquareSize)/2,height-resizeSquareSize,resizeSquareSize,resizeSquareSize),Qt::white);

    painter.fillRect(QRect(0,(height-resizeSquareSize)/2,resizeSquareSize,resizeSquareSize),Qt::white);
    painter.fillRect(QRect(width-resizeSquareSize,(height-resizeSquareSize)/2,resizeSquareSize,resizeSquareSize),Qt::white);
}


}

#include "composerimageresizewidget.moc"
