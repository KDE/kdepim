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
# pragma implementation "EmpathQuotedText.h"
#endif

// Local includes
#include "EmpathQuotedText.h"

EmpathQuotedText::EmpathQuotedText()
    :   parsed_(false),
        assembled_(false)
{
    quotedParts_.setAutoDelete(true);
}

EmpathQuotedText::EmpathQuotedText(const QString & text)
    :   parsed_(false),
        assembled_(false),
        strRep_(text)
{
    quotedParts_.setAutoDelete(true);
}

    void
EmpathQuotedText::rewrap(const uint maxLineLength)
{
    parse();
    assembled_ = false;
    
    QStringList::Iterator lit;
    QListIterator<EmpathQuotedText::Part> pit(quotedParts_);

    for (; pit.current(); ++pit) {
        
        EmpathQuotedText::Part * part = pit.current();
        uint maxLength = maxLineLength - part->quotePrefix.length();

        for (lit = part->lines.begin(); lit != part->lines.end(); ++lit) 
            
            if ((*lit).length() > maxLength) {
                
                QString line(*lit);
                
                // Find a position where we can break the line.
                int breakPos = line.findRev(' ', maxLength);
                
                // Add the two parts of the line to the list.
                *lit = line.right(line.length() - breakPos - 1);
                part->lines.insert(lit, line.left(breakPos));
                
                // Decreasing the iterator causes the new lines
                // to be processed too.
                --lit;
            }
    }
}

    void 
EmpathQuotedText::quote()
{
    parse();
    assembled_ = false;
    
    QListIterator<EmpathQuotedText::Part> pit(quotedParts_);

    for (; pit.current(); ++pit) 
        pit.current()->quotePrefix.prepend("> ");
}

    void 
EmpathQuotedText::_parse()
{
    quotedParts_.clear();

    QString quotePrefix, restOfLine;
    EmpathQuotedText::Part * part;
    int index;

    // Give QStringList::split a little help, so that it doesn't skip empty
    // lines.
    QString text(strRep_);
    text.replace(QRegExp("\n\n"), "\n \n");
    QStringList lines; // = QStringList::split('\n', text);

    QStringList::Iterator it;
    
    // Initialise a part only if there is at least one line.
    if (lines.count()) {   
        part = new EmpathQuotedText::Part;
        quotedParts_.append(part);
    }
 
    for (it = lines.begin(); it != lines.end(); ++it) {
	
        QString line(*it);
        line.replace(QRegExp("\n \n"), "\n\n"); // Revert our change.
        
        index = 0;
    
        // Find quote prefix, storing it in quotePrefix. The quote prefix
        // should be at the beginning of the line. It consists of any
        // combination of spaces and >'s, but should end with "> ".
        while ( /* index < MAX_PREFIX_LENGTH && */ index < line.length() &&
                (line[index] == ' ' || 
                 line[index] == '>' ||
                 line[index] == ':' ) ) 
		    index++;

        index = line.findRev(QRegExp("[>:] "), index);

        if (index == -1) { 
            // No quote prefix
            quotePrefix = "";
            restOfLine = line;
        }
        else {
            index += 2; // Skip the last "> "
            quotePrefix = line.left(index);
            restOfLine = line.right(line.length() - index);
        }

        if (quotePrefix != part->quotePrefix) {
        
            part = new EmpathQuotedText::Part;
            quotedParts_.append(part);
            part->quotePrefix = quotePrefix;
            
        }
           
        part->lines.append(restOfLine);
    }
}

    void 
EmpathQuotedText::_assemble()
{
    QString text;
    QStringList::Iterator lit;
    QListIterator<EmpathQuotedText::Part> pit(quotedParts_);

    for (; pit.current(); ++pit) {
        
        EmpathQuotedText::Part * part = pit.current();

        for ( lit = part->lines.begin(); lit != part->lines.end(); ++lit) {
            text += part->quotePrefix;
            text += *lit;
            text += '\n';
        }
    }

    strRep_ = text;
}

// vim:ts=4:sw=4:tw=78
