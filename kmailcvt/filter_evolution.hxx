/*
 *  filter_evolution.hxx
 *  Author : Simon MARTIN <simartin@users.sourceforge.net>
 *  Copyright (c) 2004 Simon MARTIN
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef FILTER_EVOLUTION_HXX
#define FILTER_EVOLUTION_HXX

#include "filters.hxx"

/**
 * Imports Evolution mboxes recursively, keeping the folder structure.
 * @author Simon MARTIN
 */
class FilterEvolution : public Filter
{
public:
  FilterEvolution(void);
  ~FilterEvolution(void);

  void import(FilterInfo *info);

private:
  QString mailDir;
  
  void importDirContents(FilterInfo*, const QString&, const QString&, const QString&);
  void importMBox(FilterInfo*, const QString&, const QString&, const QString&);
};

#endif
