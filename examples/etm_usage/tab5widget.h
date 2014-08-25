/*
    This file is part of Akonadi.

    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

// READ THE README FILE

#ifndef TAB5WIDGET_H
#define TAB5WIDGET_H

#include <QWidget>

class QModelIndex;
class KCategorizedView;

class EntityTreeWidget;

class Tab5Widget : public QWidget
{
    Q_OBJECT
public:
    Tab5Widget(QWidget *parent = 0, Qt::WindowFlags f = 0);

private:
    KCategorizedView *m_itemView;
    EntityTreeWidget *m_etw;

};

#endif

