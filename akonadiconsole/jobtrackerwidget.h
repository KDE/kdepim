/*
    This file is part of Akonadi.

    Copyright (c) 2009 Till Adam <adam@kde.org>

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

#ifndef AKONADICONSOLE_JOBTRACKERWIDGET_H
#define AKONADICONSOLE_JOBTRACKERWIDGET_H

#include <QWidget>

class QModelIndex;
class QFile;
class JobTrackerWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit JobTrackerWidget( const char *name, QWidget *parent, const QString& checkboxText );
    ~JobTrackerWidget();
  private slots:
    void contextMenu(const QPoint& pos);
    void slotSaveToFile();

  private:
    void writeRows( const QModelIndex &parent, QFile &file, int indentLevel );

    class Private;
    Private* const d;
};

#endif
