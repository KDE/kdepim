#ifndef _KPILOT_SYSINFO_H
#define _KPILOT_SYSINFO_H
/* sysInfo.h			KPilot
**
** Copyright (C) 2003 by Reinhold Kainhofer
**
** Wrapper for pilot-link's SysInfo Structure
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pi-version.h>
#include <pi-dlp.h>



class KPilotSysInfo
{
public:
	/** Constructor. Create an empty SysInfo structure. */
	KPilotSysInfo() 
	{
		::memset(&fSysInfo,0,sizeof(struct SysInfo)); 
	}

	/** Constructor. Copy an existing pilot-link SysInfo structure. 
	 *  Ownership is not changed. @p sys_info may be NULL.
	 */ 
	KPilotSysInfo(const SysInfo *sys_info) 
	{
		::memset(&fSysInfo,0,sizeof(struct SysInfo)); 
		if (sys_info)
		{
			fSysInfo = *sys_info;
		}
	}

	/** Access to the raw SysInfo structure. */
	SysInfo *sysInfo()
	{
		return &fSysInfo;
	}

	/** Get the ROM version of the handheld. This is a pilot-link
	 *  long value (4 bytes) with major, minor, bugfix version
	 *  numbers encoded in the value.
	 */
	const unsigned long getRomVersion() const 
	{
		return fSysInfo.romVersion;
	}

	/** Get the locale number of the handheld. 
	 *  @note I do not know what this means.
	 */
	const unsigned long getLocale() const 
	{
		return fSysInfo.locale;
	}
	/** Set the locale number of the handheld.
	 *  @note I do not know what this means.
	 */
	void setLocale(unsigned long newval)
	{
		fSysInfo.locale=newval;
	}

	/** Get the length (in bytes) of the ProductID string. */
	const int getProductIDLength() const
	{
		return fSysInfo.prodIDLength;
	}
	/** Get the ProductID string from the handheld. This is
	 *  guaranteed to be NUL terminated.
	 */
	const char* getProductID() const
	{
		return fSysInfo.prodID;
	}

	/** Accessor for the major version of the DLP protocol in use. */
	const unsigned short getMajorVersion() const 
	{
		return fSysInfo.dlpMajorVersion;
	}
	/** Accessor for the minor version of the DLP protocol in use. */
	const unsigned short getMinorVersion() const 
	{
		return fSysInfo.dlpMinorVersion;
	}

	/** Accessor for the major compatibility version of the handheld.
	 *  @note I do not know what this means.
	 */
	const unsigned short getCompatMajorVersion() const 
	{
		return fSysInfo.compatMajorVersion;
	}
	/** Accessor for the minor compatibility version of the handheld.
	 *  @note I do not know what this means.
	 */
	const unsigned short getCompatMinorVersion() const 
	{
		return fSysInfo.compatMinorVersion;
	}


	/** Returns the maximum record size that the handheld supports.
	 *  Normally this is 65524 or so (which means that larger values
	 *  don't necessarily @em fit in a short).
	 */
	const unsigned short getMaxRecSize() const 
	{
		return fSysInfo.maxRecSize;
	}

private:
	struct SysInfo fSysInfo;
};

#endif
