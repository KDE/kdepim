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

#ifndef COISCEIMWIDGET_H
#define COISCEIMWIDGET_H

#include <QWidget>

class TripModel;

class CoisceimWidget : public QWidget
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.coisceim.CoisceimWidget")
public:
  CoisceimWidget(QWidget *parent = 0);

  Q_INVOKABLE void createTrip();

private:
  TripModel *m_tripModel;
};

#endif
