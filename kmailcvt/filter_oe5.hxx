/***************************************************************************
                          filter_oe5.hxx  -  description
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

#include "filters.hxx"

#ifndef __FILTER_OE5__
#define __FILTER_OE5__

class FilterOE5 : public Filter
{
  public:
    FilterOE5();
   ~FilterOE5();
  public:
    void import(FilterInfo *info);
  private:
    QString CAP;
};

#endif
