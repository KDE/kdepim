/*
    This file is part of Akonadi.

    Copyright (c) 2011 Stephen Kelly <steveire@gmail.com>

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

#ifndef EVENTSELECTORWIDGET_H
#define EVENTSELECTORWIDGET_H

#include <KDialog>
#include <Akonadi/Item>

class QTreeView;

namespace CalendarSupport {
class IncidenceViewer;
}

class EventSelectorDialog : public KDialog
{
  Q_OBJECT
public:
  explicit EventSelectorDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

  Akonadi::Item selectedItem();

private:
  QTreeView *m_view;
};

class EventSelectorWidget : public QWidget
{
  Q_OBJECT
public:
  explicit EventSelectorWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

  void clear();

signals:
  void selected(const Akonadi::Item &item);

private slots:
  void selectTrip();

private:
  CalendarSupport::IncidenceViewer *m_browser;
};

#endif
