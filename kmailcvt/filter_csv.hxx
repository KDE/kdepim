/***************************************************************************
                          FilterCSV.hxx  -  description
                             -------------------
    begin                : Tue Feb 18, 2003
    copyright            : (C) 2003 by Laurence Anderson
    email                : l.d.anderson@warwick.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FILTER_CSV
#define FILTER_CSV

#include "filters.hxx"

class FilterCSV : public Filter,  protected FilterFactory< FilterCSV >
{
  public:
    FilterCSV();
   ~FilterCSV();
    void import(FilterInfo *info);
    bool convert(const QString &filename, FilterInfo *info);
};


#endif
