/***************************************************************************
                          FilterPAB.hxx  -  description
                             -------------------
    begin                : Fri Jun 30 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __FILTER_PAB
#define __FILTER_PAB

#include "filters.hxx"

class FilterPAB : public Filter, protected FilterFactory< FilterPAB >
{
  public:
    FilterPAB();
   ~FilterPAB();
  public:
    void import(FilterInfo *info);
};


#endif
