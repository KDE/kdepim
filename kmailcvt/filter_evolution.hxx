/*
 *  filter_evolution.hxx
 *  Author : Simon MARTIN <simartin@users.sourceforge.net>
 *  Copyright (c) 2004 Simon MARTIN <simartin@users.sourceforge.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    explicit FilterEvolution();
    ~FilterEvolution();

    void import();
private:
    QString mailDir;

    void importDirContents(const QString&, const QString&, const QString&);
    void importMBox(const QString&, const QString&, const QString&);
};

#endif
