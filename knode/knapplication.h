/***************************************************************************
                     knapplication.h - description
 copyright            : (C) 2000 by Christian Gebauer
 email                : gebauer@bigfoot.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNAPPLICATION_H
#define KNAPPLICATION_H

#include <kuniqueapp.h>


class KNApplication : public KUniqueApplication
{
  Q_OBJECT

  public:
    KNApplication();
    ~KNApplication();

    /** Create new instance of KNode. Make the existing
        main window active if KNode is already running */
    int newInstance();

};

#endif
