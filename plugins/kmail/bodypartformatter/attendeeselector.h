/*
    Copyright (c) a2007 Volker Krause <vkrause@kde.org>

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

#ifndef ATTENDEESELECTOR_H_H
#define ATTENDEESELECTOR_H_H

#include <kdialogbase.h>

class AttendeeSelectorWidget;

/**
  Dialog to select a set off attendees.
*/
class AttendeeSelector : public KDialogBase
{
  Q_OBJECT
  public:
    AttendeeSelector( QWidget *parent = 0 );

    QStringList attendees() const;

  private slots:
    void addClicked();
    void removeClicked();
    void textChanged( const QString &text );
    void selectionChanged();

  private:
    AttendeeSelectorWidget *ui;
};


#endif
