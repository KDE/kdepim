//=============================================================================
// File:       datetime.h
// Contents:   Declarations for DwDateTime
// Maintainer: Doug Sauder <dwsauder@fwb.gulf.net>
// WWW:        http://www.fwb.gulf.net/~dwsauder/mimepp.html
//
// Copyright (c) 1996, 1997 Douglas W. Sauder
// All rights reserved.
//
// IN NO EVENT SHALL DOUGLAS W. SAUDER BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF
// THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF DOUGLAS W. SAUDER
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// DOUGLAS W. SAUDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT
// NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
// BASIS, AND DOUGLAS W. SAUDER HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
// SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
//=============================================================================

#ifndef DW_DATETIME_H
#define DW_DATETIME_H

#include <time.h>

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_FIELDBDY_H
#include <mimelib/fieldbdy.h>
#endif

//=============================================================================
//+ Name DwDateTime -- Class representing an RFC-822 date-time
//+ Description
//. {\tt DwDatetime} represents a {\it date-time} as described in RFC-822
//. and RFC-1123.  The parse method for {\tt DwDateTime} parses the
//. string representation to extract the year, month, day, hour, minute,
//. second, and time zone.  {\tt DwDateTime} provides member functions
//. to set or get the individual components of the date-time.
//=============================================================================
// Last modified 1997-08-23
//+ Noentry ~DwDateTime mYear mMonth mDay mHour mMinute mSecond mZone
//+ Noentry sDefaultZone sIsDefaultZoneSet _PrintDebugInfo


class DW_EXPORT DwDateTime : public DwFieldBody {

public:

    DwDateTime();
    DwDateTime(const DwDateTime& aDateTime);
    DwDateTime(const DwString& aStr, DwMessageComponent* aParent=0);
    //. The first constructor is the default constructor, which assigns
    //. the current date and time as reported by the operating system.
    //.
    //. The second constructor is the copy constructor.  The parent of
    //. the new {\tt DwDateTime} object is set to {\tt NULL}.
    //.
    //. The third constructor sets {\tt aStr} as the {\tt DwDateTime}
    //. object's string representation and sets {\tt aParent} as its parent.
    //. The virtual member function {\tt Parse()} should be called after
    //. this constructor to extract the date and time information from the
    //. string representation.  Unless it is {\tt NULL}, {\tt aParent} should
    //. point to an object of a class derived from {\tt DwField}.

    virtual ~DwDateTime();

    const DwDateTime& operator = (const DwDateTime& aDateTime);
    //. This is the assignment operator, which sets this {\tt DwDateTime}
    //. object to the same value as {\tt aDateTime}.

    virtual void Parse();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the parse method for {\tt DwDateTime} objects. The parse
    //. method creates or updates the broken-down representation from the
    //. string representation.  For {\tt DwDateTime} objects, the parse
    //. method parses the string representation to extract the year,
    //. month, day, hour, minute, second, and time zone.
    //.
    //. This function clears the is-modified flag.

    virtual void Assemble();
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. executes the assemble method for {\tt DwDateTime} objects.
    //. It should be called whenever one of the object's attributes
    //. is changed in order to assemble the string representation from
    //. its broken-down representation.  It will be called
    //. automatically for this object by the parent object's
    //. {\tt Assemble()} member function if the is-modified flag is set.
    //.
    //. This function clears the is-modified flag.

    virtual DwMessageComponent* Clone() const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. creates a new {\tt DwDateTime} on the free store that has the same
    //. value as this {\tt DwDateTime} object.  The basic idea is that of
    //. a virtual copy constructor.

    DwUint32 AsUnixTime() const;
    //. Returns the date and time as a UNIX (POSIX) time, defined as the
    //. number of seconds elapsed since 1 Jan 1970 00:00:00 UTC.

    void FromUnixTime(DwUint32 aTime);
    //. Sets the date and time from {\tt aTime}, interpreted as the number of
    //. of seconds elapsed since 1 Jan 1970 00:00:00 UTC.

    void FromCalendarTime(time_t aTime);
    //. Sets the date and time from {\tt aTime}, which is assumed to be in a
    //. format compatible with the native {\tt time()} ANSI C function.
    //. For most UNIX systems, this function is the same as the function
    //. {\tt FromUnixTime()}.  (For efficiency, use {\tt FromUnixTime()}
    //. instead of {\tt FromCalendarTime()} if possible).

    DwInt32 DateAsJulianDayNum() const;
    //. Returns the Julian Day Number, defined as the number of days elapsed
    //. since 1 Jan 4713 BC.  The JDN is calculated directly from the values
    //. of the year, month, and day; time zone information is ignored.

    void DateFromJulianDayNum(DwInt32 aJdn);
    //. Sets the year, month, and day from {\tt aJdn}, interpreted as a Julian
    //. Day Number.  By definition, the JDN is the number of days elapsed
    //. since 1 Jan 4713 BC.  This member function ignores time zone
    //. information.

    DwInt32 TimeAsSecsPastMidnight() const;
    //. Returns the number of seconds past midnight.  The value is
    //. calculated directly from the values of the hour, minute, and
    //. second; time zone information is ignored.

    void TimeFromSecsPastMidnight(DwInt32 aSecs);
    //. Sets the hour, minute, and second from {\tt aSecs}, interpreted as the
    //. number of seconds elapsed since midnight.  This member function
    //. ignores time zone information.  The argument {\tt aSecs} should be in
    //. the range 0 to 86399, inclusive.

    int Year() const;
    //. Returns the four digit year, e.g. 1997.

    void SetYear(int aYear);
    //. Sets the year from {\tt aYear}, which should be a four digit year.

    int Month() const;
    //. Returns the month.  Values range from 1 to 12.

    void SetMonth(int aMonth);
    //. Sets the month from {\tt aMonth}, which should be in the range 1
    //. to 12.

    int Day() const;
    //. Returns the day of the month.  Values range from 1 to 31.

    void SetDay(int aDay);
    //. Sets the day of the month from {\tt aDay}.

    int Hour() const;
    //. Returns the hour according to the 24 hour clock.
    //. Values range from 0 to 23.

    void SetHour(int aHour);
    //. Sets the hour from {\tt aHour} based on the 24-hour clock. {\tt aHour}
    //. should be in the range 0 to 23.

    int Minute() const;
    //. Returns the minute.  Values range from 0 to 59.

    void SetMinute(int aMinute);
    //. Sets the minute from {\tt aMinute}, which should be in the range 0
    //. to 59.

    int Second() const;
    //. Returns the second.  Values range from 0 to 59.

    void SetSecond(int aSecond);
    //. Sets the second from {\tt aSecond}, which should be in the range 0
    //. to 59.

    int Zone() const;
    //. Returns the time zone as the diffence in minutes between local time
    //. and Coordinated Universal Time (UTC or GMT).

    void SetZone(int aZone);
    //. Sets the time zone from {\tt aZone}, interpreted as the time difference
    //. in minutes between local time and Coordinated Universal Time
    //. (UTC, or GMT).

    static void SetDefaultZone(int aZone);
    //. Sets the default time zone.  {\tt aZone} should be the time difference
    //. in minutes between local time and Coordinated Universal Time
    //. (UTC, or GMT).  The value is used to set the time zone for any
    //. objects created using the default constructor.

    static DwDateTime* NewDateTime(const DwString&, DwMessageComponent*);
    //. Creates a new {\tt DwDateTime} object on the free store.
    //. If the static data member {\tt sNewDateTime} is {\tt NULL},
    //. this member function will create a new {\tt DwDateTime}
    //. and return it.  Otherwise, {\tt NewDateTime()} will call
    //. the user-supplied function pointed to by {\tt sNewDateTime},
    //. which is assumed to return an object from a class derived from
    //. {\tt DwDateTime}, and return that object.

    //+ Var sNewDateTime
    static DwDateTime* (*sNewDateTime)(const DwString&, DwMessageComponent*);
    //. If {\tt sNewDateTime} is not {\tt NULL}, it is assumed to point to a
    //. user-supplied function that returns an object from a class derived
    //. from {\tt DwDateTime}.

protected:

    void _FromUnixTime(DwUint32 aTime);
    //. Like {\tt FromUnixTime()}, but doesn't set the is-modified flag.

    void _FromCalendarTime(time_t aTime);
    //. Like {\tt FromCalendarTime()}, but doesn't set the is-modified flag.

    int  mYear;
    int  mMonth;
    int  mDay;
    int  mHour;
    int  mMinute;
    int  mSecond;
    int  mZone;

    static int sDefaultZone;
    static int sIsDefaultZoneSet;

private:

    static const char* const sClassName;

    void Init();
    //. Initialization code common to all constructors.

public:

    virtual void PrintDebugInfo(std::ostream& aStrm, int aDepth=0) const;
    //. This virtual function, inherited from {\tt DwMessageComponent},
    //. prints debugging information about this object to {\tt aStrm}.
    //.
    //. This member function is available only in the debug version of
    //. the library.

    virtual void CheckInvariants() const;
    //. Aborts if one of the invariants of the object fails.  Use this
    //. member function to track down bugs.
    //.
    //. This member function is available only in the debug version of
    //. the library.

protected:

    void _PrintDebugInfo(std::ostream& aStrm) const;

};


inline int DwDateTime::Year() const
{
    return mYear;
}


inline int DwDateTime::Month() const
{
    return mMonth;
}


inline int DwDateTime::Day() const
{
    return mDay;
}


inline int DwDateTime::Hour() const
{
    return mHour;
}


inline int DwDateTime::Minute() const
{
    return mMinute;
}


inline int DwDateTime::Second() const
{
    return mSecond;
}


inline int DwDateTime::Zone() const
{
    return mZone;
}


inline void DwDateTime::SetYear(int aYear)
{
    mYear = aYear;
    SetModified();
}


inline void DwDateTime::SetMonth(int aMonth)
{
    mMonth = aMonth;
    SetModified();
}


inline void DwDateTime::SetDay(int aDay)
{
    mDay = aDay;
    SetModified();
}


inline void DwDateTime::SetHour(int aHour)
{
    mHour = aHour;
    SetModified();
}


inline void DwDateTime::SetMinute(int aMinute)
{
    mMinute = aMinute;
    SetModified();
}


inline void DwDateTime::SetSecond(int aSecond)
{
    mSecond = aSecond;
    SetModified();
}


inline void DwDateTime::SetZone(int aZone)
{
    mZone = aZone;
    SetModified();
}

#endif
