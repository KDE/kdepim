/* This file is part of the KDE project

   Copyright (C) 1999, 2000 Rik Hemsley <rik@kde.org>
             (C) 1999, 2000 Wilco Greven <j.w.greven@student.utwente.nl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qvaluelist.h>
#include <string.h>
#include <stddef.h>
#include <qstring.h>
#include <qstrlist.h>

#include <RMM_Mailbox.h>
#include <RMM_Defines.h>
#include <RMM_Enum.h>

namespace RMM {

    Q_UINT32
RTokenise(
    const char * str,
    const char * delim,
    QStrList & l,
    bool skipComments,
    bool quotedTokens)
{
    l.clear();
    
    if (!delim || !str || strlen(delim) == 0 || strlen(str) == 0) return 0;
    
    char * len = const_cast<char *>(str + strlen(str));    // End of string.
    
    int rl = strlen(str);
    char * rstart = new char[rl + 1];
    char * r = rstart;
    
    
    const char * i = str;        // Cursor.
    
    while (i <= len) {

        // I'm ignoring all non-printable chars.
        // Note that 127 (DEL) is ignored here. So what ? RFC822 is fucked up.
        // I'm not going to pander to its self-contradictory crap.
        if ((*i < 32 || *i == 127) && *i != '\t') {
            ++i;
            continue;
        }

        if (*i == '\\') { // Escaped chars go straight through.
            *r++ = *i++;
            if (i <= len)
                *r++ = *i++;
            continue;
        }
        
        if (strchr(delim, *i) != 0) {
            // We hit a delimiter. If we have some text, make a new token.
            // This has the effect that multiple delimiters are collapsed.
            *r = '\0';
            if (r != rstart) {
                l.append(rstart);
                r = rstart;
            }   
            ++i;
            continue;
        }
        
        // If we find a quote, we need to continue through until
        // the next quote, ignoring delimiters
        if (*i == '"') {
          
            if (quotedTokens) {

                // If there's anything in the buffer, make a token from
                // that first.
                if (r != rstart) {
                    *r = '\0';
                    l.append(rstart);
                    r = rstart;
                }
            }
            
            do {
                *r++ = *i++;
            } while (i < len - 1 && (*(i - 1) != '\\' && *i != '"'));
            
            *r++ = *i++; // End quote mark

            if (quotedTokens) {

                *r = '\0';
                l.append(rstart);
                r = rstart;
                ++i;
            }

            continue;
        }
                    
        // If open brace and not a quoted brace, do nothing until we find
        // the ending brace or the end of the string.
        if (*i == '(') {

            // If there's anything in the buffer, make a token from that first.
            if (r != rstart) {
                *r = '\0';
                l.append(rstart);
                r = rstart;
            }
    
            do {
                *r++ = *i++;
            } while (i < len && (*(i - 1) != '\\' && *i != ')'));
            
            *r++ = *i++; // Closing brace

            *r = '\0';
            
            if (!skipComments) {
                l.append(rstart);
            }
            r = rstart;
            ++i;
            continue;
        }

        *r++ = *i++;
    }

    // Catch last token
    if (r != rstart) {
        *r = '\0';
        l.append(rstart);
    }
    
    r = 0;
    
    delete [] rstart;

    return l.count();
}

} // namespace


// vim:ts=4:sw=4:tw=78
