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

#ifndef CREATETRIPWIDGET_H
#define CREATETRIPWIDGET_H

#include <QWidget>

#include <AkonadiCore/Item>

namespace Akonadi
{
class ChangeRecorder;
}

class CreateFolderContentsWidget;
class Trip;
class EventSelectorWidget;

class CreateTripWidget : public QWidget
{
  Q_OBJECT
public:
  explicit CreateTripWidget(Trip *trip, Akonadi::ChangeRecorder* monitor, QWidget* parent = 0, Qt::WindowFlags f = 0);

private slots:
  void tripSelected(const Akonadi::Item &item);
  void create();

private:
  CreateFolderContentsWidget* createView(const QString &type, int role);

  Trip *m_trip;
  Akonadi::ChangeRecorder* m_monitor;
  Akonadi::Item m_tripItem;
  CreateFolderContentsWidget *m_mailWidget;
  CreateFolderContentsWidget *m_todoWidget;
  CreateFolderContentsWidget *m_notesWidget;
  EventSelectorWidget *m_eventSelector;
};

#endif