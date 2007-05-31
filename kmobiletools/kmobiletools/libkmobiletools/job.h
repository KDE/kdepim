/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

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
#ifndef KMT_JOB_H
#define KMT_JOB_H

#include <libkmobiletools/kmobiletools_export.h>
#include <threadweaver/Job.h>

namespace KMobileTools {
class JobPrivate;
class KMOBILETOOLS_EXPORT Job : public ThreadWeaver::Job {
    Q_OBJECT
    public:
        explicit Job (const QString &owner, QObject* parent = 0);
        explicit Job (Job *pjob, QObject* parent = 0);
        ~Job();
      enum JobType
        { UserJob=255, initPhone=0, pollStatus=-1, fetchSMS=-2, fetchAddressBook=-3, fetchPhoneInfos=-4, testPhoneFeatures=-5,
        syncDateTimeJob=-6, selectSMSSlot=-7, selectCharacterSet=-8, sendSMS=-9, storeSMS=-10, sendStoredSMS=-11, addAddressee=-12, delAddressee=-13, editAddressee=-14,
        smsFolders=-15, delSMS=-16, fetchKCal=-17 };

        virtual JobType type() = 0;
        const QString typeString();
    protected:
        void setPercentDone(quint8 percentDone);
        quint8 percentDone();
    protected Q_SLOTS:
        void slotPercentDone();
        void slotPercentDone(quint8);
    private:
        JobPrivate *const d;
    Q_SIGNALS:
        void percentDone(quint8);
};

}
#endif
