/***************************************************************************
                          filter_ldif.hxx  -  description
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

#ifndef __FILTER_LDIF
#define __FILTER_LDIF

#include "filters.hxx"

class filter_ldif : public filter
{
  public:
    filter_ldif();
   ~filter_ldif();
  private:
    int codes[256];
    char* alphabet;
    void initCodeTable();
  public:
    void import(filterInfo *info);
    bool convert(const QString &filename, filterInfo *info);
    QString decodeBase64(QString aStr);
};


#endif
