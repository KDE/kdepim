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

/** This is a class to handle conversions of todos. The todos
  * can be converted from a QDomNode to a TodoSyncee* and vice versa.
  * @author Maurus Erni
  */

namespace PVHelper
{
  class Todo
  {
    public:
      /**
        * Converts a QDomNode to a TodoSyncee*.
        * @param node The node (part of an XML document) to be converted
        * @return KSync::TodoSyncee* The converted todos
        */
      static KSync::TodoSyncee* toTodoSyncee(QDomNode& n);

      /**
        * Converts a TodoSyncee* to a QString which represents a
        * DOM node.
        * @param syncee The syncee to be converted
        * @return QString The converted todos as an XML string
        */
      static QString toXML(KSync::TodoSyncee* syncee);
  };
}

#endif
