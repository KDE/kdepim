/***************************************************************************
            filter_thunderbird.hxx  -  Thunderbird mail import
                             -------------------
    begin                : Januar 26 2005
    copyright            : (C) 2005 by Danny Kukawka
    email                : danny.kukawka@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef FILTER_THUNDERBIRD_HXX
#define FILTER_THUNDERBIRD_HXX

#include "filters.hxx"

/**
 * Imports Thinderbird mail folder recursively, keeping the folder structure.
 * @author Danny Kukawka
 */
class FilterThunderbird : public Filter
{
public:
  FilterThunderbird(void);
  ~FilterThunderbird(void);

  void import(FilterInfo *info);

private:
  QString mailDir;
  
  void importDirContents(FilterInfo*, const QString&, const QString&, const QString&);
  void importMBox(FilterInfo*, const QString&, const QString&, const QString&);
};

#endif
