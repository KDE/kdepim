/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

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
*/
#ifndef ToDoHelper_H
#define ToDoHelper_H


#include <qdom.h>
#include <qmap.h>
#include <qvaluelist.h>

#include <libkcal/todo.h>

#include <kontainer.h>

#include "helper.h"

class OpieCategories;

namespace KSync {
    class KonnectorUIDHelper;
    class Syncee;
}
namespace OpieHelper {
struct TodoExtraItem : public CustomExtraItem
{
  TodoExtraItem( bool c, int co )
    : completed( c ), completion( co )
    {}

  bool completed : 1;
  int  completion;
};

class ToDo  : public Base
{
public:
  ToDo( CategoryEdit* edit = 0,
        KSync::KonnectorUIDHelper* helper= 0,
        const QString& tz = QString::null,
        Device* dev = 0);
  ~ToDo();

  bool toKDE( const QString &fileName, ExtraMap& map, KSync::CalendarSyncee* );
  KTempFile* fromKDE( KSync::CalendarSyncee* entry, ExtraMap& map  );

private:
  QStringList supportedAttributes();
  void setUid( KCal::Todo*,  const QString &uid );
  KCal::Todo* dom2todo( QDomElement, ExtraMap&, const QStringList& );
  QString todo2String( KCal::Todo*, ExtraMap&  );

/* time conversions */
private:
  static QString  dateToString( const QDate& );
  static QDate    stringToDate( const QString& );
};
}


#endif
