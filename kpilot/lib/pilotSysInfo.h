#ifndef _KPILOT_SYSINFO_H
#define _KPILOT_SYSINFO_H
/* sysInfo.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef _PILOT_DLP_H_
#include <pi-dlp.h>
#endif

class KPilotSysInfo
{
public:
	KPilotSysInfo() { ::memset(&fSysInfo,0,sizeof(struct SysInfo)); }
	KPilotSysInfo(const SysInfo* sys_info) { fSysInfo = *sys_info; }

	SysInfo *sysInfo() { return &fSysInfo; }

	/**
	* Ensures the names are properly terminated.  Needed incase we
	* are syncing a new and bogus pilot.
	*/
	void boundsCheck()
	{
	}

        const unsigned long getRomVersion() const {return fSysInfo.romVersion;}
        void setRomVersion(unsigned long newval)  {fSysInfo.romVersion=newval;}

        const unsigned long getLocale() const {return fSysInfo.locale;}
        void setLocale(unsigned long newval)  {fSysInfo.locale=newval;}

	const int getProductIDLength() const { return fSysInfo.prodIDLength; }
	const char* getProductID() const     { return fSysInfo.prodID; }
	void setProductID(char* prodid)
	{
		::memset(&fSysInfo.prodID, 0, sizeof(fSysInfo.prodID));
		::strncpy(fSysInfo.prodID, prodid, sizeof(fSysInfo.prodID)-1);
		boundsCheck();
		fSysInfo.prodIDLength = ::strlen(fSysInfo.prodID);
	}

        const unsigned short getMajorVersion() const {return fSysInfo.dlpMajorVersion;}
        const unsigned short getMinorVersion() const {return fSysInfo.dlpMinorVersion;}
        const unsigned short getCompatMajorVersion() const {return fSysInfo.compatMajorVersion;}
        const unsigned short getCompatMinorVersion() const {return fSysInfo.compatMinorVersion;}
        const unsigned short getMaxRecSize() const {return fSysInfo.maxRecSize;}
private:
	struct SysInfo fSysInfo;
};

#endif
