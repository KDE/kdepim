// $Id$	

#include <kdebug.h>
#include <klocale.h>

#include "attendee.h"

using namespace KCal;

Attendee::Attendee(const QString &name, const QString &email, bool _rsvp, Attendee::PartStat s,
                   Attendee::Role r) :
  Person(name,email)
{
  mFlag = TRUE;
  mRSVP = _rsvp;
  mStatus = s;
  mRole = r;
}

#if 0
Attendee::Attendee(const Attendee &a)
{
  flag = a.flag;
  rsvp = a.rsvp;
  status = a.status;
  role = a.role;
}
#endif

Attendee::~Attendee()
{
}


void Attendee::setStatus(Attendee::PartStat s)
{
  mStatus = s;
}

Attendee::PartStat Attendee::status() const
{
  return mStatus;
}

QString Attendee::statusStr() const
{
  return statusName(mStatus);
}

QString Attendee::statusName( Attendee::PartStat s )
{
    switch (s) {
        default:
        case NeedsAction:
            return i18n("Needs Action");
            break;
        case Accepted:
            return i18n("Accepted");
            break;
        case Declined:
            return i18n("Declined");
            break;
        case Tentative:
            return i18n("Tentative");
            break;
        case Delegated:
            return i18n("Delegated");
            break;
        case Completed:
            return i18n("Completed");
            break;
        case InProcess:
            return i18n("In Process");
            break;
    }
}

QStringList Attendee::statusList()
{
    QStringList list;
    list << statusName(NeedsAction);
    list << statusName(Accepted);
    list << statusName(Declined);
    list << statusName(Tentative);
    list << statusName(Delegated);
    list << statusName(Completed);
    list << statusName(InProcess);

    return list;
}


void Attendee::setRole(Attendee::Role r)
{
  mRole = r;
}

Attendee::Role Attendee::role() const
{
  return mRole;
}

QString Attendee::roleStr() const
{
  return roleName(mRole);
}

QString Attendee::roleName( Attendee::Role r )
{
    switch (r) {
        case Chair:
            return i18n("Chair");
            break;
        default:
        case ReqParticipant:
            return i18n("Participant");
            break;
        case OptParticipant:
            return i18n("Optional Participant");
            break;
        case NonParticipant:
            return i18n("Observer");
            break;
    }
}

QStringList Attendee::roleList()
{
    QStringList list;
    list << roleName(ReqParticipant);
    list << roleName(OptParticipant);
    list << roleName(NonParticipant);
    list << roleName(Chair);

    return list;
}
