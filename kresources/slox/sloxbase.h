/*
    Copyright (c) 2005 by Volker Krause <volker.krause@rwth-aachen.de>
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

#ifndef SLOXUTILS_H
#define SLOXUTILS_H

#include <qstring.h>
#include <kdepimmacros.h>

namespace KRES {
class Resource;
}

class KDE_EXPORT SloxBase {
  public:
    enum Field {
      ObjectId = 0, // system fields
      ClientId,
      FolderId,
      LastSync,
      ObjectType,
      ObjectStatus,
      IncidenceTitle, // incidence fields
      Description,
      Participants,
      Participant,
      Reminder,
      RecurrenceType, // recurrence fields
      RecurrenceEnd,
      RecurrenceDailyFreq,
      RecurrenceWeeklyFreq,
      RecurrenceMonthlyFreq,
      RecurrenceMonthlyDay,
      RecurrenceYearlyDay,
      RecurrenceYearlyMonth,
      RecurrenceMonthly2Freq,
      RecurrenceMonthly2Day,
      RecurrenceMonthly2Pos,
      RecurrenceYearly2Day,
      RecurrenceYearly2Pos,
      RecurrenceYearly2Month,
      RecurrenceDelEx,
      EventBegin,   // event fields
      EventEnd,
      Location,
      FullTime,
      TaskBegin,    // task fields
      TaskEnd,
      Priority,
      PercentComplete,
      FamilyName,   // contact fields
      GivenName,
      DisplayName,
      Title,
      Role,
      Organization,
      WorkPhone1,
      WorkPhone2,
      WorkMobile1,
      WorkMobile2,
      WorkFax1,
      WorkFax2,
      PrivatePhone1,
      PrivatePhone2,
      PrivateMobile1,
      PrivateMobile2,
      PrivateFax1,
      PrivateFax2,
      PrimaryEmail,
      SecondaryEmail1,
      SecondaryEmail2,
      SecondaryEmail3,
      Birthday,
      Url,
      Comment,
      Image
    };

    SloxBase( KRES::Resource *res );

    QString decodeText( const QString &text );
    QString fieldName( Field f );
    QString resType() const;
    QString boolToStr( bool b );

  private:
    KRES::Resource *mRes;
};

#endif
