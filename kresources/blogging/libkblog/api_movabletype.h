/***************************************************************************
*   Copyright (C) 2004 by Reinhold Kainhofer <reinhold@kainhofer.com>     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#ifndef MOVABLETYPEAPI_H
#define MOVABLETYPEAPI_H

#include <api_blogger.h>

namespace KBlog {

/**
Implementation for movabletypeAPI, extends the BloggerAPI quite a bit...

@author Reinhold Kainhofer
*/
class movabletypeAPI : public bloggerAPI
{
  Q_OBJECT
public:
  movabletypeAPI( const KURL &server, QObject *parent = 0L, const char *name = 0L );
  ~movabletypeAPI();
  QString interfaceName() const { return "MovableType API"; }
};

};
#endif
