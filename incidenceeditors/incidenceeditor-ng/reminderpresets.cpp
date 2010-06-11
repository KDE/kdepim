/*
    Copyright (C) 2010  Bertjan Broeksema b.broeksema@home.nl

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/
#include "reminderpresets.h"

#include <KCal/Alarm>

using namespace KCal;

typedef boost::shared_ptr<Alarm> Ptr;

namespace IncidenceEditorsNG {

namespace ReminderPresets {

// Don't use a map, because order matters
static QStringList sPresetNames;
static QList<Ptr>  sPresets = QList<Ptr>();

void initPresets()
{

}

QStringList availablePresets()
{
  if ( sPresetNames.isEmpty() )
    initPresets();

  return sPresetNames;
}


KCal::Alarm *preset( const QString &name )
{

}

int presetIndex( const KCal::Alarm &alarm )
{

}


}
}
