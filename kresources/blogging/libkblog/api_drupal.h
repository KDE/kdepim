/***************************************************************************
*   Copyright (C) 2004 by Reinhold Kainhofer <reinhold@kainhofer.com>     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#ifndef DRUPALAPI_H
#define DRUPALAPI_H

#include <api_blogger.h>

namespace KBlog {

/**
Implementation for DrupalAPI, should be the same as the BloggerAPI, but just in case...

@author Reinhold Kainhofer
*/
class drupalAPI : public bloggerAPI
{
  Q_OBJECT
public:
  drupalAPI( const KURL &server, QObject *parent = 0L, const char *name = 0L );
  ~drupalAPI();
  QString interfaceName() const { return "Drupal API"; }
  QString getFunctionName( blogFunctions type );
};

}
#endif
