/***************************************************************************
            filter_evolution_v2.hxx  -  Evolution 2.0.x mail import
                             -------------------
    begin                : Januar 26 2005
    copyright            : (C) 2005 by Danny Kukawka
                           (inspired and partly copied from filter_evolution)
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
 
#ifndef FILTER_EVOLUTION_V2_HXX
#define FILTER_EVOLUTION_V2_HXX

#include "filters.hxx"

/**
 * Imports Evolution v2.x mail folder recursively, keeping the folder structure.
 * @author Danny Kukawka
 */
class FilterEvolution_v2 : public Filter
{
public:
  FilterEvolution_v2(void);
  ~FilterEvolution_v2(void);

  void import(FilterInfo *info);

private:
  QString mailDir;
  
  void importDirContents(FilterInfo*, const QString&, const QString&, const QString&);
  void importMBox(FilterInfo*, const QString&, const QString&, const QString&);
};

#endif
