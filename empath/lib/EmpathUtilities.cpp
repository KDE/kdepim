/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathUtilities.h"
#endif

// Qt includes
#include <qstring.h>

// Local includes
#include "EmpathUtilities.h"

QString baseName(const QString & filename)
{
    return filename.mid(filename.findRev('/') + 1);
}

bool stricmp(const QString & a, const QString & b)
{
    return a.lower() == b.lower();
}

// vim:ts=4:sw=4:tw=78
