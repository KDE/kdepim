/***************************************************************************
                          filter_oe4.hxx  -  description
                             -------------------
    begin                : Thu Aug 24 2000
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

#ifndef __FILTER_OE4__
#define __FILTER_OE4__

class filter_oe4 : public filter
{
  private:
   QString CAP;
  public:
    filter_oe4();
   ~filter_oe4();
  public:
    void import(filterInfo *info);
};

#endif
