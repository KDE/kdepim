/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "task.h"

#include <libkcal/todo.h>
#include <kdebug.h>

using namespace Kolab;


KCal::Todo* Task::xmlToTask( const QString& xml )
{
  Task task;
  task.load( xml );
  KCal::Todo* todo = new KCal::Todo();
  task.saveTo( todo );
  return todo;
}

QString Task::taskToXML( KCal::Todo* todo )
{
  Task task( todo );
  return task.saveXML();
}

Task::Task( KCal::Todo* task )
{
  if ( task )
    setFields( task );
}

Task::~Task()
{
}

bool Task::loadAttribute( QDomElement& element )
{
  kdDebug() << "NYI: " << k_funcinfo << endl;
  return true;
}

bool Task::saveAttributes( QDomElement& element ) const
{
  kdDebug() << "NYI: " << k_funcinfo << endl;
  return true;
}


bool Task::loadXML( const QDomDocument& document )
{
  QDomElement top = document.documentElement();

  if ( top.tagName() != "task" ) {
    qWarning( "XML error: Top tag was %s instead of the expected task",
              top.tagName().ascii() );
    return false;
  }

  for ( QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      if ( !loadAttribute( e ) )
        // TODO: Unhandled tag - save for later storage
        kdDebug() << "Warning: Unhandled tag " << e.tagName() << endl;
    } else
      kdDebug() << "Node is not a comment or an element???" << endl;
  }

  return true;
}

QString Task::saveXML() const
{
  QDomDocument document = domTree();
  QDomElement element = document.createElement( "task" );
  element.setAttribute( "version", "1.0" );
  saveAttributes( element );
  document.appendChild( element );
  return document.toString();
}

void Task::setFields( const KCal::Todo* task )
{
  kdDebug() << "NYFI: " << k_funcinfo << endl;
  KolabBase::setFields( task );
}

void Task::saveTo( KCal::Todo* task )
{
  kdDebug() << "NYFI: " << k_funcinfo << endl;
  KolabBase::saveTo( task );
}
