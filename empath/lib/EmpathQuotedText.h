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
# pragma interface "EmpathQuotedText.h"
#endif

#ifndef EMPATH_QUOTEDTEXT_H
#define EMPATH_QUOTEDTEXT_H

// Qt includes
#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

/**
 * @short Class for handling wrapping of quoted text.
 * The design of this class resembles that of the RMM classes. 
 * @author Wilco Greven
 */
class EmpathQuotedText 
{
    public:

        struct Part {
           QString      quotePrefix;
           QStringList  lines;
        };
        
        EmpathQuotedText();
        EmpathQuotedText(const QString &);

        ~EmpathQuotedText() {}

        QString asString() { assemble(); return strRep_; }
        
        void rewrap(const uint maxLineLength);
        void quote();

        void parse() 
            { if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }
        void assemble() 
            { parse(); if (!assembled_) _assemble(); assembled_ = true; }

    private:
    
        void _parse();
        void _assemble();
      
        bool parsed_;
        bool assembled_;
        
        QList<Part> quotedParts_;
        
        QString strRep_;
};

#endif

// vim:ts=4:sw=4:tw=78
