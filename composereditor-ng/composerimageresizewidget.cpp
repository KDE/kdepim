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

    void setResizeDirectionCursor(const QPoint& pos);
    ResizeDirection resizeDirection(const QPoint& pos);

    ComposerImageResizeWidget *q;
    QWebElement imageElement;
    ResizeDirection direction;
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
    if(QRect(r.x(),r.top(),3,3).contains(pos)) {
        dir = TopLeft;
    } else if(QRect(r.x(),r.bottom(),3,3).contains(pos)) {
        dir = BottomLeft;
    } else if(QRect(r.right(),r.bottom(),3,3).contains(pos)) {
        dir = BottomRight;
    } else if(QRect(r.right(),r.top(),3,3).contains(pos)) {
        dir = TopRight;
    } else if(QRect(r.x(),r.y(),r.width(),3).contains(pos)) {
        dir = Top;
    } else if(QRect(r.x(),r.bottom(),r.width(),3).contains(pos)) {
        dir = Bottom;
    } else if(QRect(r.x(),r.y(),3,r.height()).contains(pos)) {
        dir = Left;
    } else if(QRect(r.right(),r.y(),3,r.height()).contains(pos)) {
        dir = Right;
    } else if(QRect(r.right(),r.y(),3,3).contains(pos)) {
        dir = TopLeft;
    } else {
        dir = None;
    }
    qDebug()<<" dir  "<<dir;
    return dir;
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
    } else {
        //TODO resize
    }
}

void ComposerImageResizeWidget::mousePressEvent( QMouseEvent * event )
{
    d->mousePressed = true;
}

void ComposerImageResizeWidget::mouseReleaseEvent( QMouseEvent * event )
{
    if(d->mousePressed) {
        //TODO resize
    }
    d->mousePressed = false;
}

void ComposerImageResizeWidget::paintEvent( QPaintEvent * )
{
    if(d->imageElement.isNull())
        return;

    QPainter painter(this);
    painter.drawRect(QRect(0,0,d->imageElement.geometry().width(),d->imageElement.geometry().height()));
}


}
