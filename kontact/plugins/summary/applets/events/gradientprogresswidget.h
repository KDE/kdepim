/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 * 
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef GRADIENTPROGRESSWIDGET_H
#define GRADIENTPROGRESSWIDGET_H

#include <QGraphicsWidget>

class GradientProgressWidget : public QGraphicsWidget
{
Q_OBJECT
public:
    GradientProgressWidget(int end = 0, int current = 0, QGraphicsWidget* parent = 0);

    void setEnd(int end);
    void setCurrent(int current);

    int end();
    int current();

    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0 );

private:
    int m_end;
    int m_current;
};

#endif
