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

#include <rmm/Defines.h>
#include <rmm/ParameterList.h>
#include <rmm/Parameter.h>
#include <rmm/Token.h>

using namespace RMM;

ParameterList::ParameterList()
    :   HeaderBody()
{
    // Empty.
}

ParameterList::ParameterList(const ParameterList & l)
    :   HeaderBody(l),
        list_(l.list_)
{
    // Empty.
}

ParameterList::ParameterList(const QCString & s)
    :   HeaderBody(s)
{
    // Empty.
}


ParameterList::~ParameterList()
{
    // Empty.
}

    ParameterList &
ParameterList::operator = (const ParameterList & l)
{
    if (this == &l) return *this;
    list_ = l.list_;
    HeaderBody::operator = (l);
    return *this;
}

    ParameterList &
ParameterList::operator = (const QCString & s)
{
    strRep_ = s;
    HeaderBody::operator = (s);
    return *this;
}

    bool
ParameterList::operator == (ParameterList & l)
{
    parse();
    l.parse();

    return false; // TODO
}

    void
ParameterList::_parse()
{
    list_.clear();
    
    QStrList l;
    tokenise(strRep_, ";", l, true, false);
    
    QStrListIterator it(l);
    
    for (; it.current(); ++it) {
        
        Parameter p(QCString(it.current()).stripWhiteSpace());
        p.parse();
        list_ << p;
    }
}

    void
ParameterList::_assemble()
{
    bool firstTime = true;
    
    QValueList<Parameter>::Iterator it;

    strRep_ = "";
    
    for (it = list_.begin(); it != list_.end(); ++it) {
        
        (*it).assemble();
        
        if (!firstTime) {
            strRep_ += QCString(";\n    ");
            firstTime = false;
        }

        strRep_ += (*it).asString();
    }
}

    void
ParameterList::createDefault()
{
    // STUB
}

    QValueList<Parameter>
ParameterList::list()
{
    parse();
    return list_;
}

    void
ParameterList::setList(QValueList<Parameter> & l)
{
    parse();
    list_ = l;
}

// vim:ts=4:sw=4:tw=78
