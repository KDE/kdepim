/* strToken.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** See the .cc file for an explanation of what this file is for.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/


#ifndef __STRING_TOKENIZER_H
#define __STRING_TOKENIZER_H

class StrTokenizer
    {
    public:
    StrTokenizer(const char* string, const char* delims);
    ~StrTokenizer() { delete [] fOrigString; delete [] fDelims; }
    
    const char* getNextField();
    
    protected:
    char* fOrigString;
    char* fString;
    char* fDelims;
    };

#endif


// $Log:$
