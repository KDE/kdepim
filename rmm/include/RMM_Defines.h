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

#ifndef RMM_DEFINES_H
#define RMM_DEFINES_H

#ifdef RMM_DEBUG
# include <qcstring.h>
# include <iostream.h>
# define rmmDebug(a)    cerr << className() << ": " << \
                        QCString((a)).data() << endl;
#else
# define rmmDebug(a)
#endif

#endif // Included this file
// vim:ts=4:sw=4:tw=78
