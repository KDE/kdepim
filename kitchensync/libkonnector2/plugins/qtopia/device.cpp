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

#include <addressbooksyncee.h>
#include <calendarsyncee.h>
#include <calendarmerger.h>
#include <addressbookmerger.h>

#include "device.h"

using KSync::AddressBookSyncee;
using KSync::CalendarSyncee;

using namespace OpieHelper;

Device::Device()
  : mABookMerger( 0l ), mCalendarMerger( 0l )
{
    m_model = Opie;
}

Device::~Device() {
}

int Device::distribution()const {
    return m_model;
}

void Device::setDistribution( int dist ) {
    m_model = dist;
}

KSync::Merger* Device::merger( enum PIM pim )
{
  KSync::Merger* merger;

  switch( pim ) {
    case Calendar:
      merger = opieCalendarMerger();
      break;
    case Addressbook:
      merger = opieAddressBookMerger();
      break;
    default:
      merger = 0;
  }

  return merger;
}

KSync::Merger* Device::opieCalendarMerger(){
  if ( mCalendarMerger )
    return mCalendarMerger;

  QBitArray cal( KSync::CalendarMerger::DtEnd+1 );
  cal[KSync::CalendarMerger::Organizer] = false;
  cal[KSync::CalendarMerger::ReadOnly ] = false; // we do not support the read only attribute
  cal[KSync::CalendarMerger::DtStart  ] = true;
  cal[KSync::CalendarMerger::Duration ] = true;
  cal[KSync::CalendarMerger::Float    ] = true;
  cal[KSync::CalendarMerger::Attendee ] = false;
  cal[KSync::CalendarMerger::CreatedDate ] = false;
  cal[KSync::CalendarMerger::Revision ] = false;
  cal[KSync::CalendarMerger::Description ] = true;
  cal[KSync::CalendarMerger::Summary] = true; // ( m_model == Opie );  if we're in opie mode we do support the summcaly!
  cal[KSync::CalendarMerger::Category ] = true;
  cal[KSync::CalendarMerger::Relations ] = false;
  cal[KSync::CalendarMerger::ExDates ] = false; // currently we do not support the Exception to Recurrence
  cal[KSync::CalendarMerger::Attachments ] = false;
  cal[KSync::CalendarMerger::Secrecy ] = false;
  cal[KSync::CalendarMerger::Resources ] = false; // we do not support resources
  cal[KSync::CalendarMerger::Priority ] = false; // no priority for calendcal
  cal[KSync::CalendarMerger::Alarms ] = false; // Opie/Qtopia alarms are so different in nature
  cal[KSync::CalendarMerger::Recurrence ] = true; // we do not support everything though...
  cal[KSync::CalendarMerger::Location] = true;
  cal[KSync::CalendarMerger::DtEnd ] = true;

  // todo stuff
  QBitArray todo(KSync::CalendarMerger::DueDateTime+1);
  todo[KSync::CalendarMerger::Organizer] = false;
  todo[KSync::CalendarMerger::ReadOnly] = false;
  todo[KSync::CalendarMerger::DtStart] = ( m_model == Opie );
  todo[KSync::CalendarMerger::Duration] = false;
  todo[KSync::CalendarMerger::Float] = false; // check if DueDate less components...
  todo[KSync::CalendarMerger::Attendee] = false;
  todo[KSync::CalendarMerger::CreatedDate] = false;
  todo[KSync::CalendarMerger::Revision] = false;
  todo[KSync::CalendarMerger::Description] = true;
  todo[KSync::CalendarMerger::Summary] = ( m_model == Opie );
  todo[KSync::CalendarMerger::Category] = true;
  todo[KSync::CalendarMerger::Relations] = false;
  todo[KSync::CalendarMerger::ExDates] = false;
  todo[KSync::CalendarMerger::Attachments] = false;
  todo[KSync::CalendarMerger::Secrecy] = false;
  todo[KSync::CalendarMerger::Priority] = true;
  todo[KSync::CalendarMerger::Alarms] = false;
  todo[KSync::CalendarMerger::Recurrence] = false;
  todo[KSync::CalendarMerger::Location] = false;
  todo[KSync::CalendarMerger::StartDate] = ( m_model == Opie );
  todo[KSync::CalendarMerger::Completed] = true;
  todo[KSync::CalendarMerger::Percent] = true;
  todo[KSync::CalendarMerger::StartDateTime] = false;
  todo[KSync::CalendarMerger::DueDateTime] = false;

  mCalendarMerger = new KSync::CalendarMerger(todo, cal );

  return mCalendarMerger;
}

KSync::Merger* Device::opieAddressBookMerger(){
  if ( mABookMerger )
    return mABookMerger;

  QBitArray ar(KSync::AddressBookMerger::Emails +1 );

  ar[KSync::AddressBookMerger::FamilyName] = true;
  ar[KSync::AddressBookMerger::GivenName] = true;
  ar[KSync::AddressBookMerger::AdditionalName] = true;
  ar[KSync::AddressBookMerger::Prefix ] = false;
  ar[KSync::AddressBookMerger::Suffix] = true;
  ar[KSync::AddressBookMerger::NickName] = true;
  ar[KSync::AddressBookMerger::Birthday] = true;
  ar[KSync::AddressBookMerger::HomeAddress ] = true;
  ar[KSync::AddressBookMerger::BusinessAddress]= true;
  ar[KSync::AddressBookMerger::TimeZone] = false;
  ar[KSync::AddressBookMerger::Geo ] = false;
  ar[KSync::AddressBookMerger::Title ] = false;
  ar[KSync::AddressBookMerger::Role ] = true;
  ar[KSync::AddressBookMerger::Organization ] = true;
  ar[KSync::AddressBookMerger::Note ] = true;
  ar[KSync::AddressBookMerger::Url ] = false;
  ar[KSync::AddressBookMerger::Secrecy ] = false;
  ar[KSync::AddressBookMerger::Picture ] = false;
  ar[KSync::AddressBookMerger::Sound ] = false;
  ar[KSync::AddressBookMerger::Agent ] = false;
  ar[KSync::AddressBookMerger::HomeNumbers] = true;
  ar[KSync::AddressBookMerger::OfficeNumbers] = true;
  ar[KSync::AddressBookMerger::Messenger ] = false;
  ar[KSync::AddressBookMerger::PreferredNumber ] = false;
  ar[KSync::AddressBookMerger::Voice ] = false;
  ar[KSync::AddressBookMerger::Fax ] = false;
  ar[KSync::AddressBookMerger::Cell ] = false;
  ar[KSync::AddressBookMerger::Video ] = false;
  ar[KSync::AddressBookMerger::Mailbox ] = false;
  ar[KSync::AddressBookMerger::Modem ] = false;
  ar[KSync::AddressBookMerger::CarPhone ] = false;
  ar[KSync::AddressBookMerger::ISDN ] = false;
  ar[KSync::AddressBookMerger::PCS ] = false;
  ar[KSync::AddressBookMerger::Pager ] = false;
  ar[KSync::AddressBookMerger::HomeFax] = true;
  ar[KSync::AddressBookMerger::WorkFax] = true;
  ar[KSync::AddressBookMerger::OtherTel] = false;
  ar[KSync::AddressBookMerger::Category] = true;
  ar[KSync::AddressBookMerger::Custom] = true;
  ar[KSync::AddressBookMerger::Keys] = false;
  ar[KSync::AddressBookMerger::Logo] = false;
  ar[KSync::AddressBookMerger::Email] = true;
  ar[KSync::AddressBookMerger::Emails] = true;

  mABookMerger = new KSync::AddressBookMerger( ar );

  return mABookMerger;
}

QString Device::user()const {
    if(m_model == Opie )
	return m_user;
    else
	return QString::fromLatin1("root");
}

void Device::setUser( const QString& str ){
    m_user = str;
}

QString Device::password()const {
    if(m_model == Opie )
	return m_pass;
    else
	return QString::fromLatin1("Qtopia");
}

void Device::setPassword(const QString& pass ){
    m_pass = pass;
}

void Device::setMeta( const QString& str ){
    m_meta = str;
}

QString Device::meta()const{
    return m_meta;
}
