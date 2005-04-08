/***************************************************************************
            filter_thebat.hxx  -  TheBat! mail import
                             -------------------
    begin                : April 07 2005
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
#ifndef FILTER_THEBAT_HXX
#define FILTER_THEBAT_HXX

#include "filters.hxx"

/**
 * Imports The Bat! mail folder recursively, recreating the folder structure.
 * @author Danny Kukawka
 */
class FilterTheBat : public Filter {

public:
	FilterTheBat(void);
	~FilterTheBat(void);

	void import(FilterInfo *info);

private:
	QString mailDir;

	void importDirContents(FilterInfo*, const QString&);
	void importFiles(FilterInfo*, const QString&);
};

#endif
