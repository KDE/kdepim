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

#ifndef KPIM_TRANSPORTTYPEDIALOG_H
#define KPIM_TRANSPORTTYPEDIALOG_H

#include <mailtransport/mailtransport_export.h>
#include <kdialog.h>

class KButtonGroup;

namespace KPIM {

/**
  Dialog to select the type of a new transport.
*/
class MAILTRANSPORT_EXPORT TransportTypeDialog : public KDialog
{
  public:
    /**
      Creates a new transport type selection dialog.
    */
    TransportTypeDialog( QWidget *parent = 0 );

    /**
      Returns the selected transport type.
    */
    int transportType() const;

  private:
    KButtonGroup* mButtonGroup;
};

}

#endif
