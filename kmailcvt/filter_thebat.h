/***************************************************************************
            filter_thebat.h  -  TheBat! mail import
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

#include "filters.h"

/**
 * Imports The Bat! mail folder recursively, recreating the folder structure.
 * @author Danny Kukawka
 */
class FilterTheBat : public Filter
{

public:
    explicit FilterTheBat();
    ~FilterTheBat();

    void import();

private:
    QString mailDir;

    void importDirContents( const QString&);
    void importFiles( const QString&);
};

#endif
