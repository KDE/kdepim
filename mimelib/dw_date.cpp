//=============================================================================
// File:       dw_date.cpp
// Contents:   Date parsing function
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

/*
 * For maximum code reuse, the functions in this file are written in C.
 */

#include <mimelib/config.h>
#include <mimelib/debug.h>
#include <ctype.h>
#include <time.h>


static int CommentLength(const char *str)
{
    int ch, pos, level, quoteNext, done, len;

    level = 0;
    quoteNext = 0;
    pos = 0;
    len = 0;
    ch = str[pos];
    done = 0;
    while (1) {
        switch (ch) {
        case 0:
            len = pos;
            done = 1;
            break;
        case '\\':
            quoteNext = 1;
            break;
        case '(':
            if (!quoteNext) {
                ++level;
            }
            quoteNext = 0;
            break;
        case ')':
            if (!quoteNext) {
                --level;
                if (level == 0) {
                    len = pos + 1;
                    done = 1;
                }
            }
            quoteNext = 0;
            break;
        default:
            quoteNext = 0;
        }
        if (done) {
            break;
        }
        ++pos;
        ch = str[pos];
    }
    return len;
}


/*
 * ParseRfc822Date() -- Parse a date in RFC-822 (RFC-1123) format
 *
 * If the parsing succeeds:
 *  - tms is set to contain the year, month, day, hour, minute, and second
 *  - z is set to contain the time zone in minutes offset from UTC
 *  - 0 is returned
 * If the parsing fails:
 *  - (-1) is returned
 *  - the information in tms and z is undefined
 */
#ifdef __cplusplus
extern "C"
#endif
int ParseRfc822Date(const char *str, struct tm *tms, int *z)
{
    int pos, ch, n, sgn, numZoneDigits;
    int day=1, month=0, year=1970, hour=0, minute=0, second=0, zone=0;
    int isValid = 1;

    if (!str) {
        return -1;
    }
    /*
     * Ignore optional day of the week.
     */

    /*
     * Day -- one or two digits
     */
    /* -- skip over non-digits */
    pos = 0;
    ch = str[pos];
    while (ch && !('0' <= ch && ch <= '9')) {
        if (ch == '(') {
            pos += CommentLength(&str[pos]);
        }
        else {
            ++pos;
        }
        ch = str[pos];
    }
    /* -- convert next one or two digits */
    n = -1;
    if ('0' <= ch && ch <= '9') {
        n = ch - '0';
        ++pos;
        ch = str[pos];
    }
    if ('0' <= ch && ch <= '9') {
        n *= 10;
        n += ch - '0';
        ++pos;
        ch = str[pos];
    }
    if (1 <= n && n <= 31) {
        day = n;
    }
    else {
        isValid = 0;
    }
    /*
     * Month.  Use case-insensitive string compare for added robustness
     */
    /* -- skip over chars to first possible month char */
    while (ch && !('A' <= ch && ch <= 'S') && !('a' <= ch && ch <= 's')) {
        if (ch == '(') {
            pos += CommentLength(&str[pos]);
        }
        else {
            ++pos;
        }
        ch = str[pos];
    }
    /* -- convert the month name */
    n = -1;
    switch (ch) {
    case 'A':
    case 'a':
        /* Apr */
        if ((str[pos+1] == 'p' || str[pos+1] == 'P')
            && (str[pos+2] == 'r' || str[pos+2] == 'R')) {
            n = 3;
            pos += 3;
            ch = str[pos];
        }
        /* Aug */
        else if ((str[pos+1] == 'u' || str[pos+1] == 'U')
            && (str[pos+2] == 'g' || str[pos+2] == 'G')) {
            n = 7;
            pos += 3;
            ch = str[pos];
        }
        break;
    case 'D':
    case 'd':
        /* Dec */
        if ((str[pos+1] == 'e' || str[pos+1] == 'E')
            && (str[pos+2] == 'c' || str[pos+2] == 'C')) {
            n = 11;
            pos += 3;
            ch = str[pos];
        }
        break;
    case 'F':
    case 'f':
        /* Feb */
        if ((str[pos+1] == 'e' || str[pos+1] == 'E')
            && (str[pos+2] == 'b' || str[pos+2] == 'B')) {
            n = 1;
            pos += 3;
            ch = str[pos];
        }
        break;
    case 'J':
    case 'j':
        /* Jan */
        if ((str[pos+1] == 'a' || str[pos+1] == 'A')
            && (str[pos+2] == 'n' || str[pos+2] == 'N')) {
            n = 0;
            pos += 3;
            ch = str[pos];
        }
        /* Jul */
        else if ((str[pos+1] == 'u' || str[pos+1] == 'U')
            && (str[pos+2] == 'l' || str[pos+2] == 'L')) {
            n = 6;
            pos += 3;
            ch = str[pos];
        }
        /* Jun */
        else if ((str[pos+1] == 'u' || str[pos+1] == 'U')
            && (str[pos+2] == 'n' || str[pos+2] == 'N')) {
            n = 5;
            pos += 3;
            ch = str[pos];
        }
        break;
    case 'M':
    case 'm':
        /* Mar */
        if ((str[pos+1] == 'a' || str[pos+1] == 'A')
            && (str[pos+2] == 'r' || str[pos+2] == 'R')) {
            n = 2;
            pos += 3;
            ch = str[pos];
        }
        /* May */
        else if ((str[pos+1] == 'a' || str[pos+1] == 'A')
            && (str[pos+2] == 'y' || str[pos+2] == 'Y')) {
            n = 4;
            pos += 3;
            ch = str[pos];
        }
        break;
    case 'N':
    case 'n':
        /* Nov */
        if ((str[pos+1] == 'o' || str[pos+1] == 'O')
            && (str[pos+2] == 'v' || str[pos+2] == 'V')) {
            n = 10;
            pos += 3;
            ch = str[pos];
        }
        break;
    case 'O':
    case 'o':
        /* Oct */
        if ((str[pos+1] == 'c' || str[pos+1] == 'c')
            && (str[pos+2] == 't' || str[pos+2] == 'T')) {
            n = 9;
            pos += 3;
            ch = str[pos];
        }
        break;
    case 'S':
    case 's':
        /* Sep */
        if ((str[pos+1] == 'e' || str[pos+1] == 'E')
            && (str[pos+2] == 'p' || str[pos+2] == 'P')) {
            n = 8;
            pos += 3;
            ch = str[pos];
        }
        break;
    }
    if (0 <= n && n <= 11) {
        month = n;
    }
    else {
        isValid = 0;
    }
    /*
     * Year -- two or four digits (four preferred)
     */
    /* -- skip over non-digits */
    while (ch && !('0' <= ch && ch <= '9')) {
        if (ch == '(') {
            pos += CommentLength(&str[pos]);
        }
        else {
            ++pos;
        }
        ch = str[pos];
    }
    /* -- convert up to four digits */
    n = -1;
    if ('0' <= ch && ch <= '9') {
        n = ch - '0';
        ++pos;
        ch = str[pos];
    }
    if ('0' <= ch && ch <= '9') {
        n *= 10;
        n += ch - '0';
        ++pos;
        ch = str[pos];
    }
    if ('0' <= ch && ch <= '9') {
        n *= 10;
        n += ch - '0';
        ++pos;
        ch = str[pos];
    }
    if ('0' <= ch && ch <= '9') {
        n *= 10;
        n += ch - '0';
        ++pos;
        ch = str[pos];
    }
    if (n != -1) {
    	/* Fixed year 2000 problem (fix by tony@lasernet.globalnet.co.uk) */
	if (n < 70)
		n += 2000; /* When less than 70 assume after year 2000 */
	else if (n <= 99)
		n += 1900; /* When >69 and <100 assume 1970 to 1999 */
	/* Additional check to limit valid range to 1970 to 2037 */
	if ((n >= 1970) && (n < 2038))
		year = n;
	else
		isValid = 0;
    }
    else {
        isValid = 0;
    }
    /*
     * Hour -- two digits
     */
    /* -- skip over non-digits */
    while (ch && !('0' <= ch && ch <= '9')) {
        if (ch == '(') {
            pos += CommentLength(&str[pos]);
        }
        else {
            ++pos;
        }
        ch = str[pos];
    }
    /* -- convert next one or two digits */
    n = -1;
    if ('0' <= ch && ch <= '9') {
        n = ch - '0';
        ++pos;
        ch = str[pos];
    }
    if ('0' <= ch && ch <= '9') {
        n *= 10;
        n += ch - '0';
        ++pos;
        ch = str[pos];
    }
    if (0 <= n && n <= 23) {
        hour = n;
    }
    else {
        isValid = 0;
    }
    /*
     * Minute -- two digits
     */
    /* -- scan for ':' */
    while (ch && ch != ':') {
        if (ch == '(') {
            pos += CommentLength(&str[pos]);
        }
        else {
            ++pos;
        }
        ch = str[pos];
    }
    /* -- skip over non-digits */
    while (ch && !('0' <= ch && ch <= '9')) {
        if (ch == '(') {
            pos += CommentLength(&str[pos]);
        }
        else {
            ++pos;
        }
        ch = str[pos];
    }
    /* -- convert next one or two digits */
    n = -1;
    if ('0' <= ch && ch <= '9') {
        n = ch - '0';
        ++pos;
        ch = str[pos];
    }
    if ('0' <= ch && ch <= '9') {
        n *= 10;
        n += ch - '0';
        ++pos;
        ch = str[pos];
    }
    if (0 <= n && n <= 59) {
        minute = n;
    }
    else {
        isValid = 0;
    }
    /*
     * Second (optional) -- two digits
     */
    /* -- scan for ':' or start of time zone */
    while (ch && !(ch == ':' || ch == '+' || ch == '-' || isalpha(ch))) {
        if (ch == '(') {
            pos += CommentLength(&str[pos]);
        }
        else {
            ++pos;
        }
        ch = str[pos];
    }
    /* -- get the seconds, if it's there */
    if (ch == ':') {
        ++pos;
        /* -- skip non-digits */
        ch = str[pos];
        while (ch && !('0' <= ch && ch <= '9')) {
            if (ch == '(') {
                pos += CommentLength(&str[pos]);
            }
            else {
                ++pos;
            }
            ch = str[pos];
        }
        /* -- convert next one or two digits */
        n = -1;
        if ('0' <= ch && ch <= '9') {
            n = ch - '0';
            ++pos;
            ch = str[pos];
        }
        if ('0' <= ch && ch <= '9') {
            n *= 10;
            n += ch - '0';
            ++pos;
            ch = str[pos];
        }
        if (0 <= n && n <= 59) {
            second = n;
        }
        else {
            isValid = 0;
        }
        /* -- scan for start of time zone */
        while (ch && !(ch == '+' || ch == '-' || isalpha(ch))) {
            if (ch == '(') {
                pos += CommentLength(&str[pos]);
            }
            else {
                ++pos;
            }
            ch = str[pos];
        }
    }
    else /* if (ch != ':') */ {
        second = 0;
    }
    /*
     * Time zone
     *
     * Note: According to RFC-1123, the military time zones are specified
     * incorrectly in RFC-822.  RFC-1123 then states that "military time
     * zones in RFC-822 headers carry no information."
     * Here, we follow the specification in RFC-822.  What else could we
     * do?  Military time zones should *never* be used!
     */
    sgn = 1;
    numZoneDigits = 0;
    switch (ch) {
    case '-':
        sgn = -1;
        /* fall through */
    case '+':
        ++pos;
        /* -- skip non-digits */
        ch = str[pos];
        while (ch && !('0' <= ch && ch <= '9')) {
            ++pos;
            ch = str[pos];
        }
	while( str[pos + numZoneDigits] && isdigit(str[pos + numZoneDigits] ) )
	    ++numZoneDigits;
        /* -- convert next four digits */
        n = 0;
	while ( numZoneDigits ) {
	    switch(numZoneDigits) {
	    case 4:
		if ('0' <= ch && ch <= '9') {
		    n = (ch - '0')*600;
		    ++pos;
		    ch = str[pos];
		}
		break;
	    case 3:
		if ('0' <= ch && ch <= '9') {
		    n += (ch - '0')*60;
		    ++pos;
		    ch = str[pos];
		}
		break;
	    case 2:
		if ('0' <= ch && ch <= '9') {
		    n += (ch - '0')*10;
		    ++pos;
		    ch = str[pos];
		}
		break;
	    case 1:
		if ('0' <= ch && ch <= '9') {
		    n += ch - '0';
		}
		break;
	    default:
		break;
	    }
	    --numZoneDigits;
	}
	zone = sgn*n;
	break;
    case 'U':
    case 'u':
        if (str[pos+1] == 'T' || str[pos+1] == 't') {
            zone = 0;
        }
        else {
            /* Military time zone */
            zone = 480;
        }
        break;
    case 'G':
    case 'g':
        if ((str[pos+1] == 'M' || str[pos+1] == 'm')
            && (str[pos+2] == 'T' || str[pos+2] == 't')) {
            zone = 0;
        }
        else {
            /* Military time zone */
            zone = -420;
        }
        break;
    case 'E':
    case 'e':
        if ((str[pos+1] == 'S' || str[pos+1] == 's')
            && (str[pos+2] == 'T' || str[pos+2] == 't')) {
            zone = -300;
        }
        else if ((str[pos+1] == 'D' || str[pos+1] == 'd')
            && (str[pos+2] == 'T' || str[pos+2] == 't')) {
            zone = -240;
        }
        else {
            /* Military time zone */
            zone = -300;
        }
        break;
    case 'C':
    case 'c':
        if ((str[pos+1] == 'S' || str[pos+1] == 's')
            && (str[pos+2] == 'T' || str[pos+2] == 't')) {
            zone = -360;
        }
        else if ((str[pos+1] == 'D' || str[pos+1] == 'd')
            && (str[pos+2] == 'T' || str[pos+2] == 't')) {
            zone = -300;
        }
        else {
            /* Military time zone */
            zone = -180;
        }
        break;
    case 'M':
    case 'm':
        if ((str[pos+1] == 'S' || str[pos+1] == 's')
            && (str[pos+2] == 'T' || str[pos+2] == 't')) {
            zone = -420;
        }
        else if ((str[pos+1] == 'D' || str[pos+1] == 'd')
            && (str[pos+2] == 'T' || str[pos+2] == 't')) {
            zone = -360;
        }
        else {
            /* Military time zone */
            zone = -720;
        }
        break;
    case 'P':
    case 'p':
        if ((str[pos+1] == 'S' || str[pos+1] == 's')
            && (str[pos+2] == 'T' || str[pos+2] == 't')) {
            zone = -480;
        }
        else if ((str[pos+1] == 'D' || str[pos+1] == 'd')
            && (str[pos+2] == 'T' || str[pos+2] == 't')) {
            zone = -420;
        }
        else {
            /* Military time zone */
            zone = 180;
        }
        break;
    case 'Z':
        /* Military time zone */
        zone = 0;
        break;
    default:
        /* Military time zone */
        if ('A' <= ch && ch <= 'I') {
            zone = 'A' - 1 - ch;
        }
        else if ('K' <= ch && ch <= 'M') {
            zone = 'A' - ch;
        }
        else if ('N' <= ch && ch <= 'Y') {
            zone = ch - 'N' + 1;
        }
        /* Some software doesn't set the timezone, so we default
	   to +/-0 so KMail isn't too strict. --dnaber@mini.gt.owl.de, 2000-06-11
	else {
            isValid = 0;
        } */
        break;
    }
    if (isValid) {
        if (tms) {
            tms->tm_year = year - 1900;
            tms->tm_mon  = month;
            tms->tm_mday = day;
            tms->tm_hour = hour;
            tms->tm_min  = minute;
            tms->tm_sec  = second;
        }
        if (z) {
            *z = zone;
        }
    }
    else {
        if (tms) {
            tms->tm_year = 70;
            tms->tm_mon  = 0;
            tms->tm_mday = 1;
            tms->tm_hour = 0;
            tms->tm_min  = 0;
            tms->tm_sec  = 0;
        }
        if (z) {
            *z = 0;
        }
    }
    return isValid ? 0 : -1;
}


#ifdef DW_TESTING_DATEPARSER

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

const char* testStr[] = {
    ""
};

const char* wdays[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

const char* months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

int main()
{
    struct tm *ptms, tms1, tms2;
    time_t tt;
    int i, zone1, zone2;
    char buf[100], sgn;

    /* try a bunch of random dates */
    srand(100);
    for (i=0; i < 1000; ++i) {
        tt = rand()*((double)0x7fffffff/RAND_MAX);
        zone1 = (rand()%49 - 24)*30;
        gmtime(&tt, &ptms);
        tms1 = *ptms;
        sgn = (zone1 >= 0) ? '+' : '-';
        snprintf(buf, sizeof(buf), "%s, %2d %s %d %d%d:%d%d:%d%d %c%d%d%d%d",
            wdays[tms1.tm_wday], tms1.tm_mday, months[tms1.tm_mon],
            tms1.tm_year+1900,
            tms1.tm_hour/10, tms1.tm_hour%10,
            tms1.tm_min/10, tms1.tm_min%10,
            tms1.tm_sec/10, tms1.tm_sec%10,
            sgn, abs(zone1)/60/10, abs(zone1)/60%10,
            abs(zone1)%60/10, abs(zone1)%60%10);
        ParseRfc822Date(buf, &tms2, &zone2);
        if (tms1.tm_year != tms2.tm_year) {
            fprintf(stderr, "Bad year\n");
        }
        if (tms1.tm_mon != tms2.tm_mon) {
            fprintf(stderr, "Bad month\n");
        }
        if (tms1.tm_mday != tms2.tm_mday) {
            fprintf(stderr, "Bad day\n");
        }
        if (tms1.tm_hour != tms2.tm_hour) {
            fprintf(stderr, "Bad hour\n");
        }
        if (tms1.tm_min != tms2.tm_min) {
            fprintf(stderr, "Bad minute\n");
        }
        if (tms1.tm_sec != tms2.tm_sec) {
            fprintf(stderr, "Bad second\n");
        }
        if (zone1 != zone2) {
            fprintf(stderr, "Bad zone\n");
        }
    }
    return 0;
}

#endif
