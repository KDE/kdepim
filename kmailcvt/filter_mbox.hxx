/***************************************************************************
                          filter_mbox.hxx  -  mbox mail import
                             -------------------
    begin                : Sat Apr 5 2003
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

#ifndef FILTER_MBOX_HXX
#define FILTER_MBOX_HXX

#include "filters.hxx"

/**imports mbox archives messages into KMail
 *@author Laurence Anderson
 */

class FilterMBox : public Filter, protected FilterFactory< FilterMBox >  {
  public:
    FilterMBox();
    ~FilterMBox();

    void import(FilterInfo *info);
};

#endif

// vim: ts=2 sw=2 et
