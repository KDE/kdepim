/* This file is part of the KDE libraries
   Copyright (C) 2004 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kasciistricmp.h"

#if ! KDE_IS_VERSION(3,3,89)
int kdepim_kasciistricmp( const char *str1, const char *str2 )
{
    const unsigned char *s1 = (const unsigned char *)str1;
    const unsigned char *s2 = (const unsigned char *)str2;
    int res;
    unsigned char c1, c2;

    if ( !s1 || !s2 )
        return s1 ? 1 : (s2 ? -1 : 0);
    if ( !*s1 || !*s2 )
        return *s1 ? 1 : (*s2 ? -1 : 0);
    for (;*s1; ++s1, ++s2) {
        c1 = *s1; c2 = *s2;
        if (c1 >= 'A' && c1 <= 'Z')
            c1 += 'a' - 'A';
        if (c2 >= 'A' && c2 <= 'Z')
            c2 += 'a' - 'A';

        if ((res = c1 - c2))
            break;
    }
    return res;
}
#endif
