/* This file is part of the KDE project
 * Copyright (C) 2004 Bram Schoenmakers <bramschoenmakers@kde.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that
t it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <tqdatetime.h>
#include <tqfile.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/todo.h>

#include "kfile_ics.h"

#include <kgenericfactory.h>

using namespace KCal;

typedef KGenericFactory<ICSPlugin> ICSFactory;
K_EXPORT_COMPONENT_FACTORY(kfile_ics, ICSFactory( "kfile_ics" ))

ICSPlugin::ICSPlugin( TQObject *parent, const char *name, const TQStringList& args )
  : KFilePlugin( parent, name, args )
{
  KFileMimeTypeInfo* info = addMimeTypeInfo( "text/calendar" ); //TODO: vcs !!

  KFileMimeTypeInfo::GroupInfo* group = 0L;
  group = addGroupInfo(info, "ICSInfo", i18n("Calendar Statistics"));

  addItemInfo( group, "ProductID",   i18n("Product ID"),     TQVariant::String );
  addItemInfo( group, "Events",        i18n("Events"),         TQVariant::Int );
  addItemInfo( group, "Todos",         i18n("To-dos"),         TQVariant::Int );
  addItemInfo( group, "TodoCompleted", i18n("Completed To-dos"), TQVariant::Int );
  addItemInfo( group, "TodoOverdue",   i18n("Overdue To-dos"), TQVariant::Int );
  addItemInfo( group, "Journals",      i18n("Journals"),       TQVariant::Int );
//   addItemInfo( group, "Reminders",     i18n("Reminders"),      TQVariant::Int );

}

/*
I chose to use libkcal instead of reading the calendar manually. It's easier to
maintain this way.
*/
bool ICSPlugin::readInfo( KFileMetaInfo& info, uint /*what*/ )
{
  KFileMetaInfoGroup group = appendGroup( info, "ICSInfo");

  CalendarLocal cal ( TQString::fromLatin1( "UTC" ) );
  if( !cal.load( info.path() ) ) {
    kdDebug() << "Could not load calendar" << endl;
    return false;
  }

  appendItem( group, "ProductID", TQVariant( cal.productId() ) );
  appendItem( group, "Events", TQVariant( int( cal.events().count() ) ) );
  appendItem( group, "Journals", TQVariant( int( cal.journals().count() ) ) );
  Todo::List todos = cal.todos();

  // count completed and overdue
  Todo::List::ConstIterator it = todos.begin();
  Todo::List::ConstIterator end = todos.end();

  int completed = 0;
  int overdue = 0;
  for ( ; it != end ; ++it ) {
    Todo *todo = *it;
    if ( todo->isCompleted() )
      ++completed;
    else if ( todo->hasDueDate() && todo->dtDue().date() < TQDate::currentDate() )
      ++overdue;
  }

  appendItem( group, "Todos", TQVariant( int(todos.count() ) ) );
  appendItem( group, "TodoCompleted", TQVariant( completed ) );
  appendItem( group, "TodoOverdue", TQVariant( overdue ) );

  cal.close();

  return true;
}

#include "kfile_ics.moc"
