/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KMOBILETOOLSJOBXP_H
#define KMOBILETOOLSJOBXP_H

#include "kmobiletools_export.h"

#include <KDE/ThreadWeaver/Job>

namespace KMobileTools {

class JobXPPrivate;
/**
 * This class describes a job that should be used by
 * engines as base class for all jobs they need to perform
 * (e.g. fetching of address book or sms )
 *
 * If you want allow your job to be aborted, implement "void requestAbort()"
 * as public slot.
 *
 * @author Matthias Lechner <matths@lmme.de>
 */
class KMOBILETOOLS_EXPORT JobXP : public ThreadWeaver::Job
{
public:
    enum Type { // fetch jobs
                fetchAddressbook = 0,
                fetchInformation = 1,
                fetchShortMessages = 2,
                fetchSMSFolders = 3,
                fetchSMSCenter = 4,
                fetchStatusInformation = 5,

                // modification jobs
                addAddressee = 100,
                editAddressee = 101,
                removeAddressee = 102,
                storeSMS = 103,
                sendSMS = 104,
                removeSMS = 105,
                addSMSFolder = 106,
                editSMSFolder = 107,
                removeSMSFolder = 108,

                custom = 999
    };

    /**
     * Creates a new job of type @p jobType with parent @p parent
     *
     * @param jobType the type of the created job
     * @param parent the parent
     */
    JobXP( JobXP::Type jobType, QObject* parent = 0 );
    ~JobXP();

    /**
     * Returns the type of this job
     *
     * @return the job type
     */
    JobXP::Type jobType() const;

    bool canBeAborted() const;

private:
    JobXPPrivate* const d;

};

}

#endif
