/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef GAMMUSMS_H
#define GAMMUSMS_H

#include "sms.h"

/**
 * This class extends the SMS class by gammu specific features
 *
 * @author Matthias Lechner
 */
class GammuSMS : public SMS {
    public:
        explicit GammuSMS( QObject *parent = 0, const char *name = 0 );

        void setLocation( int location ) { m_location = location; }
        int location() { return m_location; }

    private:
        int m_location;
};

#endif
