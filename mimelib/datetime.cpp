//=============================================================================
// File:       datetime.cpp
// Contents:   Definitions for DwDateTime
// Maintainer: Doug Sauder <dwsauder@fwb.gulf.net>
// WWW:        http://www.fwb.gulf.net/~dwsauder/mimepp.html
// $Revision$
// $Date$
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

#define DW_IMPLEMENTATION

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <mimelib/config.h>
#include <mimelib/debug.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <mimelib/string.h>
#include <mimelib/datetime.h>
#include <mimelib/token.h>
#include <time.h>

static char lWeekDay[7][4]
    = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static char lMonth[12][4]
    = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

extern "C" int ParseRfc822Date(const char *str, struct tm *tms, int *z);
static DwInt32 ymd_to_jdnl(int year, int mon, int day, int julian);
static void jdnl_to_ymd(DwInt32 jdn, int *year, int *mon, int *day, int julian);
static DwUint32 my_inv_gmtime(struct tm* ptms);

const char* const DwDateTime::sClassName = "DwDateTime";


int DwDateTime::sDefaultZone = 0;
int DwDateTime::sIsDefaultZoneSet = 0;
DwDateTime* (*DwDateTime::sNewDateTime)(const DwString&,
    DwMessageComponent*) = 0;


DwDateTime* DwDateTime::NewDateTime(const DwString& aStr,
    DwMessageComponent* aParent)
{
    if (sNewDateTime) {
        return sNewDateTime(aStr, aParent);
    }
    else {
        return new DwDateTime(aStr, aParent);
    }
}


void DwDateTime::SetDefaultZone(int aZone)
{
    sDefaultZone = aZone;
    sIsDefaultZoneSet = 1;
}


DwDateTime::DwDateTime()
{
    Init();
    mIsModified = 1;
}


DwDateTime::DwDateTime(const DwDateTime& aDateTime)
  : DwFieldBody(aDateTime)
{
    mYear   = aDateTime.mYear;
    mMonth  = aDateTime.mMonth;
    mDay    = aDateTime.mDay;
    mHour   = aDateTime.mHour;
    mMinute = aDateTime.mMinute;
    mSecond = aDateTime.mSecond;
    mZone   = aDateTime.mZone;
}


DwDateTime::DwDateTime(const DwString& aStr, DwMessageComponent* aParent)
  : DwFieldBody(aStr, aParent)
{
    Init();
    mIsModified = 0;
}


void DwDateTime::Init()
{
    mClassId = kCidDateTime;
    mClassName = DwDateTime::sClassName;
    // Check if default time zone is set
    if (sIsDefaultZoneSet == 0) {
        // Use calls to gmtime() and localtime() to get the time difference
        // between local time and UTC (GMT) time.
        time_t t_now = time((time_t*) 0);
#if defined(HAVE_GMTIME_R)
        struct tm utc;
        gmtime_r(&t_now, &utc);
        struct tm local;
        localtime_r(&t_now, &local);
#else
        struct tm utc = *gmtime(&t_now);
        struct tm local = *localtime(&t_now);
#endif
        DwUint32 t_local = my_inv_gmtime(&local);
        DwUint32 t_utc = my_inv_gmtime(&utc);
        sDefaultZone = (int) (t_local - t_utc)/60;
        sIsDefaultZoneSet = 1;
    }
    // Set the time zone from the default time zone
    mZone = sDefaultZone;
    // Get the current calendar time
    time_t t_now = time((time_t*) 0);
    // Set year, month, day, hour, minute, and second from calendar time
    _FromCalendarTime(t_now);
}


DwDateTime::~DwDateTime()
{
}


const DwDateTime& DwDateTime::operator = (const DwDateTime& aDateTime)
{
    if (this == &aDateTime) return *this;
    DwFieldBody::operator = (aDateTime);
    mYear   = aDateTime.mYear;
    mMonth  = aDateTime.mMonth;
    mDay    = aDateTime.mDay;
    mHour   = aDateTime.mHour;
    mMinute = aDateTime.mMinute;
    mSecond = aDateTime.mSecond;
    mZone   = aDateTime.mZone;
    return *this;
}


DwUint32 DwDateTime::AsUnixTime() const
{
    struct tm tt;
    tt.tm_year = mYear - 1900;
    tt.tm_mon  = mMonth - 1;
    tt.tm_mday = mDay;
    tt.tm_hour = mHour;
    tt.tm_min  = mMinute;
    tt.tm_sec  = mSecond;
    DwUint32 t = my_inv_gmtime(&tt);
    t = (t == (DwUint32) -1) ? 0 : t;
    t -= mZone*60;
    return t;
}


void DwDateTime::FromUnixTime(DwUint32 aTime)
{
    _FromUnixTime(aTime);
    SetModified();
}


void DwDateTime::_FromUnixTime(DwUint32 aTime)
{
    time_t t = aTime + mZone*60;
#if defined(HAVE_GMTIME_R)
    struct tm tt;
    gmtime_r(&t, &tt);
#else
    struct tm tt = *gmtime(&t);
#endif
    mYear   = tt.tm_year + 1900;
    mMonth  = tt.tm_mon + 1;
    mDay    = tt.tm_mday;
    mHour   = tt.tm_hour;
    mMinute = tt.tm_min;
    mSecond = tt.tm_sec;
}

void DwDateTime::FromCalendarTime(time_t aTime)
{
    _FromCalendarTime(aTime);
    SetModified();
}


void DwDateTime::_FromCalendarTime(time_t aTime)
{
    // Note: the broken-down time is the only portable representation.
    // ANSI does not even require that time_t be an integer type; it could
    // be a double.  And, it doesn't even have to be in seconds.

    // Get the broken-down time.
#if defined(HAVE_GMTIME_R)
    struct tm tms_utc;
    gmtime_r(&aTime, &tms_utc);
#else
    struct tm tms_utc = *gmtime(&aTime);
#endif
    // Convert to UNIX time, using portable routine
    DwUint32 t_unix = my_inv_gmtime(&tms_utc);
    // Set from the UNIX time
    _FromUnixTime(t_unix);
}


DwInt32 DwDateTime::DateAsJulianDayNum() const
{
    DwInt32 jdn = ymd_to_jdnl(mYear, mMonth, mDay, -1);
    return jdn;
}


void DwDateTime::DateFromJulianDayNum(DwInt32 aJdn)
{
    jdnl_to_ymd(aJdn, &mYear, &mMonth, &mDay, -1);
    SetModified();
}


DwInt32 DwDateTime::TimeAsSecsPastMidnight() const
{
    DwInt32 n = mHour;
    n *= 60;
    n += mMinute;
    n *= 60;
    n += mSecond;
    return n;
}


void DwDateTime::TimeFromSecsPastMidnight(DwInt32 aSecs)
{
    mSecond = (int) (aSecs % 60);
    aSecs /= 60;
    mMinute = (int) (aSecs % 60);
    aSecs /= 60;
    mHour = (int) (aSecs % 24);
    SetModified();
}


void DwDateTime::Parse()
{
    mIsModified = 0;
    char buffer[80];
    char *str;
    int mustDelete;
    // Allocate memory from heap only in rare instances where the buffer
    // is too small.
    if (mString.length() >= 80) {
        mustDelete = 1;
        str = new char [mString.length()+1];
    }
    else {
        mustDelete = 0;
        str = buffer;
    }
    strncpy(str, mString.data(), mString.length());
    str[mString.length()] = 0;
    str[79] = 0;
    struct tm tms;
    int zone;
    int err = ParseRfc822Date(str, &tms, &zone);
    if (!err) {
        mYear   = tms.tm_year + 1900;
        mMonth  = tms.tm_mon+1;
        mDay    = tms.tm_mday;
        mHour   = tms.tm_hour;
        mMinute = tms.tm_min;
        mSecond = tms.tm_sec;
        mZone   = zone;
    }
    else /* if (err) */ {
        mYear   = 1970;
        mMonth  = 1;
        mDay    = 1;
        mHour   = 0;
        mMinute = 0;
        mSecond = 0;
        mZone   = 0;
    }
    if (mustDelete) {
        delete str;
    }
}


void DwDateTime::Assemble()
{
    if (!mIsModified) return;
    // Find the day of the week
    DwInt32 jdn = DateAsJulianDayNum();
    int dow = (int) ((jdn+1)%7);
    char sgn = (mZone < 0) ? '-' : '+';
    int z = (mZone < 0) ? -mZone : mZone;
    char buffer[80];
    snprintf(buffer, sizeof(buffer), "%s, %d %s %4d %02d:%02d:%02d %c%02d%02d",
        lWeekDay[dow], mDay, lMonth[(mMonth-1)%12], mYear,
        mHour, mMinute, mSecond, sgn, z/60%24, z%60);
    mString = buffer;
    mIsModified = 0;
}


DwMessageComponent* DwDateTime::Clone() const
{
    return new DwDateTime(*this);
}


#if defined (DW_DEBUG_VERSION)
void DwDateTime::PrintDebugInfo(std::ostream& aStrm, int /*aDepth*/) const
{
    aStrm <<
    "---------------- Debug info for DwDateTime class ---------------\n";
    _PrintDebugInfo(aStrm);
}
#else
void DwDateTime::PrintDebugInfo(std::ostream& , int) const {}
#endif // defined (DW_DEBUG_VERSION)


#if defined (DW_DEBUG_VERSION)
void DwDateTime::_PrintDebugInfo(std::ostream& aStrm) const
{
    DwFieldBody::_PrintDebugInfo(aStrm);
    aStrm << "Date:             "
        << mYear << '-' << mMonth << '-' << mDay << ' '
        << mHour << ':' << mMinute << ':' << mSecond << ' '
        << mZone << '\n';
}
#else
void DwDateTime::_PrintDebugInfo(std::ostream& ) const {}
#endif // defined (DW_DEBUG_VERSION)


void DwDateTime::CheckInvariants() const
{
#if defined (DW_DEBUG_VERSION)
    DwFieldBody::CheckInvariants();
    assert(mYear >= 0);
    assert(1 <= mMonth && mMonth <= 12);
    assert(1 <= mDay && mDay <= 31);
    assert(0 <= mHour && mHour < 24);
    assert(0 <= mMinute && mMinute < 60);
    assert(0 <= mSecond && mSecond < 60);
    assert(-12*60 <= mZone && mZone <= 12*60);
#endif // defined (DW_DEBUG_VERSION)
}


#ifdef PAPAL                    /* Pope Gregory XIII's decree */
#define LASTJULDATE 15821004L   /* last day to use Julian calendar */
#define LASTJULJDN  2299160L    /* jdn of same */
#else                           /* British-American usage */
#define LASTJULDATE 17520902L   /* last day to use Julian calendar */
#define LASTJULJDN  2361221L    /* jdn of same */
#endif


static DwInt32 ymd_to_jdnl(int year, int mon, int day, int julian)
{
    DwInt32 jdn;

    if (julian < 0)         /* set Julian flag if auto set */
        julian = (((year * 100L) + mon) * 100 + day  <=  LASTJULDATE);

    if (year < 0)              /* adjust BC year */
        year++;

    if (julian)
        jdn = 367L * year - 7 * (year + 5001L + (mon - 9) / 7) / 4
        + 275 * mon / 9 + day + 1729777L;
    else
        jdn = (DwInt32)(day - 32075)
            + 1461L * (year + 4800L + (mon - 14) / 12) / 4
            + 367 * (mon - 2 - (mon - 14) / 12 * 12) / 12
            - 3 * ((year + 4900L + (mon - 14) / 12) / 100) / 4;

    return jdn;
}


static void jdnl_to_ymd(DwInt32 jdn, int *year, int *mon, int *day, int julian)
{
    DwInt32 x, z, m, d, y;
    DwInt32 daysPer400Years = 146097L;
    DwInt32 fudgedDaysPer4000Years = 1460970L + 31;

    if (julian < 0)                 /* set Julian flag if auto set */
        julian = (jdn <= LASTJULJDN);

    x = jdn + 68569L;
    if (julian) {
        x += 38;
        daysPer400Years = 146100L;
        fudgedDaysPer4000Years = 1461000L + 1;
    }
    z = 4 * x / daysPer400Years;
    x = x - (daysPer400Years * z + 3) / 4;
    y = 4000 * (x + 1) / fudgedDaysPer4000Years;
    x = x - 1461 * y / 4 + 31;
    m = 80 * x / 2447;
    d = x - 2447 * m / 80;
    x = m / 11;
    m = m + 2 - 12 * x;
    y = 100 * (z - 49) + y + x;

    *year = (int)y;
    *mon = (int)m;
    *day = (int)d;

    if (*year <= 0)                   /* adjust BC years */
        (*year)--;
}

#define JDN_JAN_1_1970  2440588L

/*
 * Converts broken-down time to time in seconds since 1 Jan 1970 00:00.
 * Pays no attention to time zone or daylight savings time.  Another way
 * to think about this function is that it is the inverse of gmtime().
 * One word of caution: the values in the broken down time must be
 * correct.
 *
 * This function is different from mktime() in three ways:
 * 1. mktime() accepts a broken-down local time and converts it to a scalar
 *    UTC time.  Thus, mktime() takes time zone and daylight savings time
 *    information into account when computing the scalar time.  (This makes
 *    mktime() highly non-portable).
 * 2. mktime() will adjust for non-standard values, such as a tm_mday member
 *    that is out of range.  This function does no such conversion.
 * 3. mktime() sets the struct fields tm_yday, tm_wday, and tm_isdst to
 *    their correct values on output.  This function does not.
 */
static DwUint32 my_inv_gmtime(struct tm* ptms)
{
    DwInt32 jdn;
    DwUint32 t;

    jdn = ymd_to_jdnl(ptms->tm_year+1900, ptms->tm_mon+1,
        ptms->tm_mday, -1);
    t = jdn - JDN_JAN_1_1970;  /* days    */
    t = 24*t + ptms->tm_hour;  /* hours   */
    t = 60*t + ptms->tm_min;   /* minutes */
    t = 60*t + ptms->tm_sec;   /* seconds */
    return t;
}


