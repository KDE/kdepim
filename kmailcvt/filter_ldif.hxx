/***************************************************************************
                          FilterLDIF.hxx  -  description
                             -------------------
    begin                : Fri Dec 1, 2000
    copyright            : (C) 2000 by Oliver Strutynski
    email                : olistrut@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FILTER_LDIF
#define FILTER_LDIF

#include "filters.hxx"

class FilterLDIF : public Filter,  protected FilterFactory< FilterLDIF >
{
  public:
    FilterLDIF();
   ~FilterLDIF();
    void import(FilterInfo *info);
    bool convert(const QString &filename, FilterInfo *info);
};


#endif
