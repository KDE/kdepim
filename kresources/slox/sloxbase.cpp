/*
    Copyright (c) 2005 by Volker Krause <vkrause@kde.org>
    Copyright (c) 2005 by Florian Schröder <florian@deltatauchi.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "sloxbase.h"

#include <kdebug.h>
#include <kresources/resource.h>

static QString mFieldNameMap[][2] =
{
  // SLOX, OX
  {"sloxid", "object_id"}, // system fields
  {"clientid", "client_id"},
  {"folderid", "folder_id"},
  {"lastsync", "lastsync"},
  {"objecttype", "objectmode"},
  {"sloxstatus", "object_status"},
  {"createfrom", "created_by"},
  {"categories", "categories"},
  // incidence fields
  {"title", "title"},
  {"description", "note"},
  {"members", "participants"},
  {"member", "user"},
  {"reminder", "alarm"},
  // recurrence fields
  {"date_sequence", "recurrence_type"},
  {"ds_ends", "until"},
  {"daily_value", "interval"},
  {"weekly_value", "interval"},
  {"monthly_value_month", "interval"},
  {"monthly_value_day", "day_in_month"},
  {"yearly_value_day", "day_in_month"},
  {"yearly_month", "month"},
  {"monthly2_value_month", "interval"},
  {"monthly2_day", "days"},
  {"monthly2_recurrency", "day_in_month"},
  {"yearly2_day", "days"},
  {"yearly2_reccurency", "day_in_month"}, // this is not a typo, this is what SLOX erally sends!
  {"yearly2_month", "month"},
  {"deleteexceptions", "deleteexceptions"},
  // event fields
  {"begins", "start_date"},
  {"ends", "end_date"},
  {"location", "location"},
  {"full_time", "full_time"},
  // task fields
  {"startdate", "start_date"},
  {"deadline", "end_date"},
  {"priority", "priority"},
  {"status", "percent_complete"},
  // contact fields
  {"lastname", "last_name"},
  {"firstname", "first_name"},
  {"n/a", "second_name"},
  {"displayname", "displayname"}, // FIXME: what's this in SLOX?
  {"title", "title"},
  {"n/a", "suffix"},
  {"position", "position"},
  {"n/a", "company"}, // SLOX handles company separately
  {"department", "department"},
  {"email", "email1"},
  {"email2", "email2"},
  {"privateemail", "email3"},
  {"privateemail2", "email3"}, // OX has only three email fields
  {"birthday", "birthday"},
  {"privateurl", "url"},
  {"comment", "note"},
  {"n/a", "image1"}, // not supported by SLOX
  {"n/a", "instant_messenger"},
  {"n/a", "room_number"},
  {"n/a", "profession"},
  {"n/a", "managers_name"},
  {"n/a", "assistants_name"},
  {"n/a", "spouse_name"},
  {"n/a", "anniversary"},
  {"n/a", "nickname"},
  {"street", "street"}, // address fields
  {"zipcode", "postal_code"},
  {"city", "city"},
  {"state", "state"},
  {"country", "country"},
  {"private", ""}, // address type prefix
  {"business_", "business_"}, // doesn't work with SLOX
  {"second_", "second_"},
};

SloxBase::SloxBase( KRES::Resource * res ) :
  mRes( res )
{
}

QString SloxBase::decodeText( const QString & text )
{
  if ( mRes->type() == "ox" )
    return text;
  return QString::fromUtf8( text.toLatin1() );
}

QString SloxBase::fieldName( Field f )
{
  int t = 0;
  if ( mRes->type() == "ox" )
    t = 1;
  return mFieldNameMap[f][t];
}

QString SloxBase::resType( ) const
{
  return mRes->type();
}

QString SloxBase::boolToStr( bool b )
{
  if ( mRes->type() == "ox" ) {
    if ( b )
      return "true";
    return "false";
  }
  if ( b )
    return "yes";
  return "no";
}
