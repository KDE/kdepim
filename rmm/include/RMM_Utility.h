/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
# pragma interface "RMM_Utility.h"
#endif

#ifndef RMM_UTILITY_H
#define RMM_UTILITY_H

#include <RMM_Enum.h>

namespace RMM {
    
QCString    toCrLfEol    (const QCString &);
QCString    toLfEol        (const QCString &);
QCString    toCrEol        (const QCString &);

QCString    encodeBase64    (const char *, unsigned long, unsigned long &);
char *      decodeBase64    (const QCString &, unsigned long &);

QCString    encodeQuotedPrintable    (const QCString &);
QCString    decodeQuotedPrintable    (const QCString &);

}
#endif
// vim:ts=4:sw=4:tw=78
