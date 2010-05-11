/*
    This file is part of KOrganizer.

    Copyright (C) 2010  Bertjan Broeksema <b.broeksema@home.nl>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef INCIDENCEGENERALEDITOR_H
#define INCIDENCEGENERALEDITOR_H

#include <QtGui/QWidget>

namespace Ui {
class IncidenceGeneral;
}

class IncidenceGeneralEditor : public QWidget
{
  Q_OBJECT
  protected: /// Methods
    /**
     * Disable creation of plain IncidenceGeneralEditor widgets. Use one of the
     * sub-classes instead.
     */
    explicit IncidenceGeneralEditor( QWidget *parent = 0 );


  protected: /// Members
    Ui::IncidenceGeneral *mUi;
};

class EventGeneralEditor : public IncidenceGeneralEditor
{
  Q_OBJECT
  public:
    explicit EventGeneralEditor( QWidget *parent = 0 );


  private:

};

class TodoGeneralEditor : public IncidenceGeneralEditor
{
  Q_OBJECT
  public:
    explicit TodoGeneralEditor( QWidget *parent = 0 );


  private:

};

#endif // INCIDENCEGENERALEDITOR_H
