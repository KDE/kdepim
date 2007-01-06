/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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

#ifndef KPIM_TRANSPORTCOMBO_H
#define KPIM_TRANSPORTCOMBO_H

#include <mailtransport/mailtransport_export.h>
#include <kcombobox.h>

namespace KPIM {

/**
  A combo-box for selecting a mail transport.
*/
class MAILTRANSPORT_EXPORT TransportComboBox : public KComboBox
{
  Q_OBJECT

  public:
    /**
      Creates a new mail transport selection combo box.
      @param parent The paren widget.
    */
    TransportComboBox( QWidget *parent = 0 );

    /**
      Returns identifier of the currently selected mail transport.
    */
    int currentTransportId() const;

    /**
      Selects the given transport.
      @param transportId The transport identifier.
    */
    void setCurrentTransport( int transportId );

  private slots:
    void fillComboBox();

  private:
    QList<int> mTransports;

};

}

#endif
