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
# pragma implementation "RMM_Cte.h"
#endif

#include <qstring.h>

#include <RMM_Cte.h>

using namespace RMM;

RCte::RCte()
    :    RHeaderBody()
{
    // Empty.
}

RCte::RCte(const RCte & cte)
    :    RHeaderBody(cte)
{
    // Empty.
}

RCte::RCte(const QCString & s)
    :    RHeaderBody(s)
{
    // Empty.
}

RCte::~RCte()
{
    // Empty.
}

    RCte &
RCte::operator = (const RCte & cte)
{
    if (this == &cte) return *this; // Don't do a = a.

    mechanism_ = cte.mechanism_;
    
    RHeaderBody::operator = (cte);
    
    return *this;
}

    RCte &
RCte::operator = (const QCString & s)
{
    RHeaderBody::operator = (s);
    return *this;
}

    bool
RCte::operator == (RCte & c)
{
    parse();
    c.parse();

    return (mechanism_ == c.mechanism_);
}

    void
RCte::_parse()
{
    strRep_        = strRep_.stripWhiteSpace();
    
    if (!stricmp(strRep_, "7bit"))
        mechanism_ = CteType7bit;
    else if (!stricmp(strRep_, "8bit"))
        mechanism_ = CteType8bit;
    else if (!stricmp(strRep_, "base64"))
        mechanism_ = CteTypeBase64;
    else if (!stricmp(strRep_, "quoted-printable"))
        mechanism_ = CteTypeQuotedPrintable;
    else if (!strnicmp(strRep_, "x", 1))
        mechanism_ = CteTypeXtension;
    else 
        mechanism_ = CteTypeBinary;
}

    void
RCte::_assemble()
{
    switch (mechanism_) {

        case CteType7bit:
            strRep_ = "7bit";
            break;
            
        case CteType8bit:
            strRep_ = "8bit";
            break;
        
        case CteTypeBase64:
            strRep_ = "Base64";
            break;
        
        case CteTypeQuotedPrintable:
            strRep_ = "Quoted-Printable";
            break;
        
        case CteTypeXtension:
            break;
        
        case CteTypeBinary:
        default:
            strRep_ = "binary";
            break;
    }
}

    void
RCte::createDefault()
{
    mechanism_    = CteTypeBase64;
    parsed_        = true;
    assembled_    = false;
}


    CteType
RCte::mechanism()
{
    parse();
    return mechanism_;
}

    void
RCte::setMechanism(CteType t)
{
    parse();
    mechanism_ = t;
}

// vim:ts=4:sw=4:tw=78
