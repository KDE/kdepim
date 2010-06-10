/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef INCIDENCERECURRENCEDIALOG_H
#define INCIDENCERECURRENCEDIALOG_H

#include <KDialog>

namespace KCal {
class Recurrence;
class Incidence;
}

namespace IncidenceEditorsNG {

class IncidenceEditor;
class IncidenceRecurrenceEditor;

class IncidenceRecurrenceDialog : public KDialog
{
  Q_OBJECT
  public:
    IncidenceRecurrenceDialog( QWidget *parent = 0 );
    ~IncidenceRecurrenceDialog();

    void load( const KCal::Recurrence &rec, const QDateTime &from, const QDateTime &to );
    void save( KCal::Recurrence *rec );
    void setDefaults( const QDateTime &from, const QDateTime &to );

  private:
    class Private;
    Private *d;

    Q_DISABLE_COPY( IncidenceRecurrenceDialog );
};

} // IncidenceEditorsNG

#endif // INCIDENCERECURRENCEDIALOG_H
