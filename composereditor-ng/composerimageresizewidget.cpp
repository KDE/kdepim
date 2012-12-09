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
#include <QPaintEvent>

namespace ComposerEditorNG
{

class ComposerImageResizeWidgetPrivate
{
public:
    ComposerImageResizeWidgetPrivate(ComposerImageResizeWidget *qq, const QWebElement& element)
        : q(qq), imageElement(element)
    {
    }
    ComposerImageResizeWidget *q;
    QWebElement imageElement;
};

ComposerImageResizeWidget::ComposerImageResizeWidget(const QWebElement &element, QWidget *parent)
    : QWidget(parent), d(new ComposerImageResizeWidgetPrivate(this,element))
{
}

ComposerImageResizeWidget::~ComposerImageResizeWidget()
{
    delete d;
}

void ComposerImageResizeWidget::mouseMoveEvent( QMouseEvent * event )
{

}

void ComposerImageResizeWidget::mousePressEvent( QMouseEvent * event )
{

}

void ComposerImageResizeWidget::mouseReleaseEvent( QMouseEvent * event )
{

}
void ComposerImageResizeWidget::paintEvent( QPaintEvent * )
{

}


}
