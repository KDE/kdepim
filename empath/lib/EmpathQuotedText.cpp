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
#include "EmpathDefines.h"
#include "EmpathQuotedText.h"

EmpathQuotedText::EmpathQuotedText()
    :   parsed_(false),
        assembled_(false)
{
    // Empty.
}

EmpathQuotedText::EmpathQuotedText(const QString & text)
    :   parsed_(false),
        assembled_(false),
        strRep_(text)
{
    // Empty.
}

    void
EmpathQuotedText::rewrap(uint maxLineLength)
{
    parse();

    QValueList<EmpathQuotedText::Part>::Iterator pit;

    for (pit = quotedParts_.begin(); pit != quotedParts_.end(); ++pit) {
        
        EmpathQuotedText::Part & part = *pit;

        uint maxLength = maxLineLength - (part.depth + part.depth);

        QStringList wrapped;

        QString overflow;
        
        QStringList::Iterator lit;

        for (lit = part.lines.begin(); lit != part.lines.end(); ++lit)  {
            
            QString line(overflow + *lit);

            if (line.length() > maxLength) {
                
                int breakPos = line.findRev(' ', maxLength);

                if (breakPos != -1) {
                
                    overflow = line.mid(breakPos + 1);

                    wrapped << line.left(breakPos);

                } else {

                    wrapped << line;
                    overflow = QString::null;
                }

            } else {

                wrapped << line;
                overflow = QString::null;
            }
        }
        
        (*pit).lines = wrapped;
    }
}

    void 
EmpathQuotedText::quote()
{
    parse();
    
    QValueList<EmpathQuotedText::Part>::Iterator pit;

    for (pit = quotedParts_.begin(); pit != quotedParts_.end(); ++pit) 
        ++((*pit).depth);
}

    void 
EmpathQuotedText::_parse()
{
    QRegExp endOfQuotes("[^> ]");

    quotedParts_.clear();

    QStringList lines = QStringList::split('\n', strRep_, true);

    QStringList tempLines;

    int oldDepth(-1);

    bool firstTime(true);

    QStringList::ConstIterator it;

    for (it = lines.begin(); it != lines.end(); ++it) {
	
        QString line(*it);
        
        empathDebug("Line        : `" + line + "'");

        int quoteEnd = line.find(endOfQuotes);
        empathDebug("quote end 1 : " + QString::number(quoteEnd));

        quoteEnd = line.findRev('>', quoteEnd);
        empathDebug("quote end 2 : " + QString::number(quoteEnd));

        if (quoteEnd == -1)
            quoteEnd = 0;

        QString quoting = line.left(quoteEnd);

        int depth = quoting.contains('>');

        if ((depth != oldDepth) && !firstTime) {

            Part part;
            part.depth = oldDepth;
            part.lines = tempLines;
            quotedParts_ << part;
        }

        empathDebug("depth       : " + QString::number(depth));
        empathDebug("remainder   : `" + line.mid(quoteEnd == 0 ? 0 : quoteEnd + 2) + "'");
        tempLines << line.mid(quoteEnd == 0 ? 0 : quoteEnd + 2);
        
        oldDepth = depth;

        firstTime = false;
    }
 
    Part part;
    part.depth = oldDepth;
    part.lines = tempLines;
    quotedParts_ << part;
}

    void 
EmpathQuotedText::_assemble()
{
    strRep_ = QString::null;

    QStringList::ConstIterator lit;
    
    QValueList<EmpathQuotedText::Part>::ConstIterator pit;

    for (pit = quotedParts_.begin(); pit != quotedParts_.end(); ++pit) {

        QString quoting;

        for (unsigned int i = 0; i < (*pit).depth; i++)
            quoting += "> ";

        for (lit = (*pit).lines.begin(); lit != (*pit).lines.end(); ++lit)
            strRep_ += quoting + *lit + '\n';
    }
}

// vim:ts=4:sw=4:tw=78
