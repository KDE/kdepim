/*
    This file is part of KDE.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
    
    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <tqcstring.h>
#include <tqdom.h>
#include <tqdatetime.h>
#include <tqstring.h>
#include <tqptrlist.h>

class ResultElementArray;
class DirectoryCategoryArray;
class GoogleSearchResult;
class DirectoryCategory;
class ResultElement;

class Serializer
{
  public:
    static TQString marshalValue( const TQString* value );
    static void demarshalValue( const TQString &str, TQString *value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const TQString* value );
    static void demarshal( const TQDomElement &element, TQString* value );
    static TQString marshalValue( const bool* value );
    static void demarshalValue( const TQString &str, bool *value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const bool* value );
    static void demarshal( const TQDomElement &element, bool* value );
    static TQString marshalValue( const float* value );
    static void demarshalValue( const TQString &str, float *value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const float* value );
    static void demarshal( const TQDomElement &element, float* value );
    static TQString marshalValue( const int* value );
    static void demarshalValue( const TQString &str, int *value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const int* value );
    static void demarshal( const TQDomElement &element, int* value );
    static TQString marshalValue( const unsigned int* value );
    static void demarshalValue( const TQString &str, unsigned int *value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const unsigned int* value );
    static void demarshal( const TQDomElement &element, unsigned int* value );
    static TQString marshalValue( const double* value );
    static void demarshalValue( const TQString &str, double *value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const double* value );
    static void demarshal( const TQDomElement &element, double* value );
    static TQString marshalValue( const char* value );
    static void demarshalValue( const TQString &str, char *value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const char* value );
    static void demarshal( const TQDomElement &element, char* value );
    static TQString marshalValue( const unsigned char* value );
    static void demarshalValue( const TQString &str, unsigned char *value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const unsigned char* value );
    static void demarshal( const TQDomElement &element, unsigned char* value );
    static TQString marshalValue( const short* value );
    static void demarshalValue( const TQString &str, short *value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const short* value );
    static void demarshal( const TQDomElement &element, short* value );
    static TQString marshalValue( const TQByteArray* value );
    static void demarshalValue( const TQString &str, TQByteArray *value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const TQByteArray* value );
    static void demarshal( const TQDomElement &element, TQByteArray* value );
    static TQString marshalValue( const TQDateTime* value );
    static void demarshalValue( const TQString &str, TQDateTime *value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const TQDateTime* value );
    static void demarshal( const TQDomElement &element, TQDateTime* value );
    static TQString marshalValue( const TQDate* value );
    static void demarshalValue( const TQString &str, TQDate *value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const TQDate* value );
    static void demarshal( const TQDomElement &element, TQDate* value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const ResultElementArray* value );
    static void demarshal( const TQDomElement &parent, ResultElementArray* value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const DirectoryCategoryArray* value );
    static void demarshal( const TQDomElement &parent, DirectoryCategoryArray* value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const GoogleSearchResult* value );
    static void demarshal( const TQDomElement &parent, GoogleSearchResult* value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const DirectoryCategory* value );
    static void demarshal( const TQDomElement &parent, DirectoryCategory* value );
    static void marshal( TQDomDocument &doc, TQDomElement &parent, const TQString &name, const ResultElement* value );
    static void demarshal( const TQDomElement &parent, ResultElement* value );
  
};

#endif
