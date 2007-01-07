/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

    Based on KMail code by:
    Copyright (C) 2001-2003 Marc Mutz <mutz@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef MAILTRANSPORT_TRANSPORTTYPEDIALOG_H
#define MAILTRANSPORT_TRANSPORTTYPEDIALOG_H

#include <mailtransport/mailtransport_export.h>
#include <kdialog.h>

class KButtonGroup;

namespace MailTransport {

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
