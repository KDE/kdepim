/***************************************************************************
                           todo.h  -  description
                             -------------------
    begin                : Wed Oct 23 2002
    copyright            : (C) 2002 by Maurus Erni
    email                : erni@pocketviewer.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef todo_h
#define todo_h

#include <qdom.h>

#include <libkcal/todo.h>
#include <todosyncee.h>

namespace PVHelper
{
  class Todo
  {
    public:
      static KSync::TodoSyncee* toTodoSyncee(QDomNode& n);

      static QString toXML(KSync::TodoSyncee* syncee);
  };
}

#endif
