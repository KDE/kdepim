/***************************************************************************
            filter_sylpheed.h  -  Sylpheed maildir mail import
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
#ifndef FILTER_SYLPHEED_HXX
#define FILTER_SYLPHEED_HXX

#include <QHash>

#include "filters.h"

/**
 * Imports Sylpheed mail folder with maildir format recursively, recreating the folder structure.
 * @author Danny Kukawka
 */
class FilterSylpheed : public Filter
{

public:
    explicit FilterSylpheed();
    ~FilterSylpheed();

    void import();

private:
    QString mailDir;

    void importDirContents(const QString&);
    void importFiles(const QString&);
    
    void readMarkFile( const QString&, QHash<QString,unsigned long>&);
    QString msgFlagsToString(unsigned long);
};

#endif
